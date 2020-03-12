#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"DirManagement.h"
#include"SimpleSandbox.h"
#include"SessionManage.h"
#include<wchar.h>
#include"VBuffer.h"
#include"RemoveCQEscapeChar.h"
#include"HandleBOITCode.h"
#include"MessageWatch.h"
#include"EncodeConvert.h"
#include"Corpus.h"
#include"Command_run.h"

#define COMPILECMD_MAXLEN 512
#define COMPILE_MAXSUFFIX 9
#define COMPILE_MAXNAME 16

#define COMPILE_TYPE_NULL 0
#define	COMPILE_TYPE_COMPILE 1
#define COMPILE_TYPE_SCRIPT 2

#define COMPILE_ENCODE_GB18030 0
#define COMPILE_ENCODE_ANSI 1
#define COMPILE_ENCODE_UTF8 2


typedef struct __tagCompileCfg
{
	int Type;
	WCHAR SourceSuffix[COMPILE_MAXSUFFIX + 1];
	WCHAR Application[MAX_PATH + 1];
	WCHAR Command[COMPILECMD_MAXLEN + 1];
	WCHAR Name[COMPILE_MAXNAME + 1];
	int SourceEncode;
	int OutputEncode;
}COMPILE_CFG, * pCOMPILE_CFG;


typedef struct __tagCompileSession
{
	pBOIT_SESSION boitSession;
	pVBUF StdOutBuffer;
	pBOIT_COMMAND Command;
	pCOMPILE_CFG CompileCfg;
	LONGLONG AllocCompileID;
	BOOL bIsSU;
	BOOL bNeedInput;
}COMPILE_SESSION, * pCOMPILE_SESSION;

typedef struct __tagRunSession
{
	UINT Encode;
	pBOIT_SESSION boitSession;
	pVBUF StdOutBuffer;
}RUN_SESSION, * pRUN_SESSION;

typedef struct __tagInputSession
{
	WCHAR Application[MAX_PATH + 1];
	WCHAR Command[COMPILECMD_MAXLEN + 1];
	WCHAR CuurentDir[MAX_PATH];
	BOOL bLimitPrivileges;
	pBOIT_SESSION boitSession;
	int Encode;
}INPUT_SESSION, * pINPUT_SESSION;






pBOIT_COMMAND pRunCmd;// 存下来给savecode什么的用

BOOL FindCompileConfig(pBOIT_COMMAND pCmd, WCHAR* LanguageName, int LanguageLen, WCHAR* ConfigSuffix, pCOMPILE_CFG CompileCfg);

BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg, WCHAR* LanguageName, int LanguageLen);

BOOL ShowSupportLanguageInfo(pBOIT_COMMAND pCmd, WCHAR* ConfigSuffix, pBOIT_SESSION boitSession);

BOOL CheckPrivilegeRunCode(long long GroupID, long long QQID);

int CompileSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen);

int GetLineLen(WCHAR* String);

int GetLineFeedLen(WCHAR* String);

BOOL GetCompileCommand(WCHAR* CommandBuffer, pCOMPILE_CFG CompileCfg, LONGLONG AllocCompileID);

int GetLineSpaceLen(WCHAR* String);

UINT GetEncodeCodePage(int Compile_Encode);

int InputCallback(long long MsgWatchID, PBYTE pData, UINT Event,
	long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);



BOOL InitSandboxDir(LONGLONG QQID, LONGLONG AllocCompileID, WCHAR* ToCopyFile, WCHAR* ToCopyFileName, WCHAR* SandboxDir, WCHAR* SandboxFile);

pSANDBOX StartRunSandbox(WCHAR* Application, WCHAR* CommandLine, WCHAR* CuurentDir, BOOL bLimitPrivileges, pBOIT_SESSION boitSession, int Encode, WCHAR* Input);


LONGLONG CompileID;

int CmdMsg_run_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	return RunCode(boitSession, Msg + ParamLen + SpaceLen);
}


int RunCode(pBOIT_SESSION orgboitSession, WCHAR* Msg)
{
	if (CheckPrivilegeRunCode(orgboitSession->GroupID, orgboitSession->QQID) == 0)
	{
		SendBackMessage(orgboitSession, Corpus_NoPrivilege());
		return 0;
	}

	//检查用户目录下是否有相应文件夹
	if (PerUserCreateDirIfNExist(orgboitSession->QQID, L"Sandbox"))
	{
		//创建相应文件
	}
	if (PerUserCreateDirIfNExist(orgboitSession->QQID, L"Compile"))
	{
		//创建相应文件
	}


	pBOIT_COMMAND pCmd = pRunCmd;
	//TODO:在这里读取用户信息，是否有权限执行程序，是否有权限以su执行程序

	int MsgLen = wcslen(Msg);
	WCHAR* lpwcParam = Msg;
	int ParamCnt = 0;
	pCOMPILE_CFG CompileCfg = malloc(sizeof(COMPILE_CFG));
	ZeroMemory(CompileCfg, sizeof(COMPILE_CFG));
	BOOL LanguageMatched = 0, bCodeFound = 0;
	WCHAR* CodeStr = 0;

	BOOL bFailed = 0, bIsSU = 0, bIsHelp = 0, bNeedInput = 0;

	WCHAR FailedReason[128];

	pBOIT_SESSION boitSession;

	boitSession = DuplicateBOITSession(orgboitSession);

	__try
	{
		for (; MsgLen > 0; ParamCnt++)
		{
			int ParamLen = GetCmdParamLen(lpwcParam);


			//处理这个 Param
			if (ParamCnt == 0 &&
				FindCompileConfig(pCmd, lpwcParam, ParamLen, L".cfg", CompileCfg) == TRUE)
			{
				LanguageMatched = TRUE;
			}
			else if (ParamCnt >= 0)
			{
				//解析可能的参数
				if (lpwcParam[0] == '-' || lpwcParam[0] == '/' || lpwcParam[0] == '\\')
				{
					int ParamLen = GetCmdParamLen(lpwcParam);
					if (ParamLen == 1)
					{
						swprintf_s(FailedReason, _countof(FailedReason), L"无法解析参数 %lc", lpwcParam[0]);
						SendBackMessage(boitSession, FailedReason);
						bFailed = 1;
						break;
					}
					else if (ParamLen >= 16)
					{
						SendBackMessage(boitSession, L"参数名称过长");
						bFailed = 1;
						break;
					}
					else
					{
						if ((ParamLen == wcslen(L"su") + 1) && (_wcsnicmp(lpwcParam + 1, L"su", wcslen(L"su")) == 0))
						{
							bIsSU = TRUE;
						}
						else if ((ParamLen == wcslen(L"help") + 1) && (_wcsnicmp(lpwcParam + 1, L"help", wcslen(L"help")) == 0))
						{
							bIsHelp = TRUE;
						}
						else if ((ParamLen == wcslen(L"input") + 1) && (_wcsnicmp(lpwcParam + 1, L"input", wcslen(L"input")) == 0))
						{
							bNeedInput = TRUE;
						}
						else
						{
							WCHAR ParamBuf[16] = { 0 };
							wcsncpy_s(ParamBuf, _countof(ParamBuf), lpwcParam, ParamLen);
							swprintf_s(FailedReason, _countof(FailedReason), L"无法解析参数 %ls", ParamBuf);
							SendBackMessage(boitSession, FailedReason);
							bFailed = 1;
							break;
						}
					}
				}
				else
				{
					//当作程序代码解析
					bCodeFound = 1;
					CodeStr = lpwcParam;
					break;
				}
			}

			lpwcParam += ParamLen;
			MsgLen -= ParamLen;
			int SpaceLen = GetCmdSpaceLen(lpwcParam);
			lpwcParam += SpaceLen;
			MsgLen -= SpaceLen;
		}
	}
	__finally
	{
		if (!bFailed)
		{
			if (!bIsHelp)
			{
				if (LanguageMatched)
				{
					if (bCodeFound)
					{
						//撒花！
					}
					else
					{
						SendBackMessage(boitSession, L"未找到源代码");
						bFailed = TRUE;
					}
				}
				else
				{
					SendBackMessage(boitSession, L"未找到语言类型或该语言不受支持");
					bFailed = TRUE;
				}
			}
		}


		if (bFailed || bIsHelp)
		{
			if (bIsHelp)
			{
				//显示详细帮助信息
				SendBackMessage(boitSession, L"usage: #run language [/su] [/input] sourcecode");
				SendBackMessage(boitSession, L"example:\n\n#run c\n#include<stdio.h>\nint main(){\nprintf(\"hello, bot\");\n}");
				SendBackMessage(boitSession, L"使用 /su 以用管理员权限执行指令。使用 /input 以启用输入数据");
				ShowSupportLanguageInfo(pCmd, L".cfg", boitSession);
			}
			else
			{
				SendBackMessage(boitSession, L"usage: #run language [/su] [/input] sourcecode\n 输入#run /help 查看详细帮助信息");
			}
			FreeBOITSession(boitSession);
			free(CompileCfg);
			return 0;
		}
	}



	//去除转义码
	RemoveCQEscapeChar(CodeStr);



	//校验权限
	if (bIsSU && (CheckUserToken(boitSession->QQID, L"PrivilegeRunCodeNoRestrict") == 0))
	{
		SendBackMessage(boitSession, Corpus_NoPrivilege());
		FreeBOITSession(boitSession);
		free(CompileCfg);
		return 0;
	}

	LONGLONG AllocCompileID = InterlockedIncrement64(&CompileID);
	//写入源代码文件

	WCHAR SourceCodeFile[MAX_PATH];

	GetPerUserDir(SourceCodeFile, boitSession->QQID);
	PathAppendW(SourceCodeFile, L"Compile\\");

	WCHAR SourceFileName[16];

	swprintf_s(SourceFileName, _countof(SourceFileName), L"Temp%lld%ls", AllocCompileID, CompileCfg->SourceSuffix);

	PathAppendW(SourceCodeFile, SourceFileName);

	HANDLE hSourceFile = CreateFile(SourceCodeFile, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	PBYTE MultiByteStrCode = 0;

	BOOL bFileCreated = FALSE;


	__try
	{
		UINT CodePage = GetEncodeCodePage(CompileCfg->SourceEncode);

		int MultiByteStrLen;
		MultiByteStrCode = StrConvWC2MB(CodePage, CodeStr, -1, &MultiByteStrLen);

		if (!MultiByteStrCode)
		{
			__leave;
		}
		
		DWORD BytesWritten;
		WriteFile(hSourceFile, (LPCVOID)MultiByteStrCode, MultiByteStrLen, &BytesWritten, 0);
		if (BytesWritten != MultiByteStrLen)
		{
			__leave;
		}

		bFileCreated = TRUE;
	}
	__finally
	{
		CloseHandle(hSourceFile);
		if (MultiByteStrCode)free(MultiByteStrCode);

		if (!bFileCreated)
		{
			SendBackMessage(boitSession, L"写入文件时出现错误");
			FreeBOITSession(boitSession);
			free(CompileCfg);
			return 0;
		}
	}


	

	switch (CompileCfg->Type)
	{
	case COMPILE_TYPE_COMPILE:
	{
		WCHAR ExecutableCodeFile[MAX_PATH];
		WCHAR ExecutableFileName[16];


		GetPerUserDir(ExecutableCodeFile, boitSession->QQID);
		PathAppendW(ExecutableCodeFile, L"Compile\\");
		swprintf_s(ExecutableFileName, _countof(ExecutableFileName), L"Temp%lld.exe", AllocCompileID);
		PathAppendW(ExecutableCodeFile, ExecutableFileName);

		if (PathFileExistsW(ExecutableCodeFile))
		{
			DeleteFile(ExecutableCodeFile);
		}

		WCHAR CompileCmd[COMPILECMD_MAXLEN + 1];
		GetCompileCommand(CompileCmd, CompileCfg, AllocCompileID);

		WCHAR WorkDir[MAX_PATH];
		GetPerUserDir(WorkDir, boitSession->QQID);
		PathAppendW(WorkDir, L"Compile\\");

		//TODO; 这个Session是临时瞎写的
		pCOMPILE_SESSION CompileSession = malloc(sizeof(COMPILE_SESSION));
		ZeroMemory(CompileSession, sizeof(COMPILE_SESSION));
		CompileSession->boitSession = boitSession;
		CompileSession->StdOutBuffer = AllocVBuf();
		CompileSession->CompileCfg = CompileCfg;
		CompileSession->Command = pRunCmd;
		CompileSession->AllocCompileID = AllocCompileID;
		CompileSession->bIsSU = bIsSU;
		CompileSession->bNeedInput = bNeedInput;

		//TODO:这些限制都是我临时写的
		if (CreateSimpleSandboxW(NULL,
			CompileCmd,
			WorkDir,
			10000000 * 10,		//10秒
			256 * 1024 * 1024,	//256MB内存
			20 * 100,				//10% CPU
			FALSE,				//不设置权限限制
			(PBYTE)CompileSession,
			CompileSandboxCallback) == 0)
		{
			SendBackMessage(boitSession, L"为编译器创建沙盒时出现意外");
			free(CompileCfg);
			FreeBOITSession(boitSession);
			free(CompileSession);
		}
	}
	break;
	case COMPILE_TYPE_SCRIPT:
	{
		WCHAR SandboxDir[MAX_PATH];
		WCHAR SandboxFile[MAX_PATH];
		InitSandboxDir(boitSession->QQID, AllocCompileID, SourceCodeFile, SourceFileName, SandboxDir, SandboxFile);

		WCHAR CompileCmd[COMPILECMD_MAXLEN + 1];
		GetCompileCommand(CompileCmd, CompileCfg, AllocCompileID);



		if (bNeedInput)
		{
			pINPUT_SESSION InputSession = malloc(sizeof(INPUT_SESSION));
			ZeroMemory(InputSession, sizeof(INPUT_SESSION));
			if (CompileCfg->Application[0]) wcscpy_s(InputSession->Application, MAX_PATH, CompileCfg->Application);
			if (CompileCmd[0])wcscpy_s(InputSession->Command, MAX_PATH, CompileCmd);
			wcscpy_s(InputSession->CuurentDir, MAX_PATH, SandboxDir);
			InputSession->bLimitPrivileges = !bIsSU;
			InputSession->Encode = CompileCfg->OutputEncode;
			InputSession->boitSession = boitSession;
			if (boitSession->GroupID)
			{
				RegisterMessageWatch(BOIT_MW_GROUP_QQ, 10000000 * 20, boitSession, InputCallback, (PBYTE)InputSession);
			}
			else
			{
				RegisterMessageWatch(BOIT_MW_QQ, 10000000 * 20, boitSession, InputCallback, (PBYTE)InputSession);
			}
			SendBackMessage(boitSession, L"请输入输入数据：");

			free(CompileCfg);
			// boitSess仍在使用中
		}
		else
		{
			if (!StartRunSandbox(CompileCfg->Application[0] ? CompileCfg->Application : NULL,
				CompileCmd, SandboxDir, !bIsSU, boitSession, CompileCfg->OutputEncode, 0))
			{
				FreeBOITSession(boitSession);//如果成功的话，这个session会继续传递下去
			}

			free(CompileCfg);

		}

	}

	break;
	}

	return 0;
}




BOOL CheckPrivilegeRunCode(long long GroupID, long long QQID)
{
	if (CheckUserToken(QQID, L"PrivilegeRunCode") == 0 || CheckGroupToken(GroupID, L"PrivilegeRunCode") == 0)
	{
		return FALSE;
	}
	return TRUE;
}


int CompileSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen)
{
	pCOMPILE_SESSION Session = (pCOMPILE_SESSION)pData;
	switch (Event)
	{
	case SANDBOX_EVTNT_PROCESS_ZERO:
	{
		DWORD CompileExitCode;
		GetExitCodeProcess(Sandbox->hProcess, &CompileExitCode);

		WCHAR ExecutableCodeFile[MAX_PATH];
		WCHAR ExecutableFileName[16];
		GetPerUserDir(ExecutableCodeFile, Session->boitSession->QQID);
		PathAppendW(ExecutableCodeFile, L"Compile\\");
		swprintf_s(ExecutableFileName, _countof(ExecutableFileName), L"Temp%lld.exe", Session->AllocCompileID);
		PathAppendW(ExecutableCodeFile, ExecutableFileName);

		if (Sandbox->ExitReason == SANDBOX_ER_EXIT && PathFileExistsW(ExecutableCodeFile)) //不对返回值进行检查，有的编译器乱返回
		{
			//清理原有的目录
			//WCHAR SandboxFile[MAX_PATH] = { 0 };
			WCHAR SandboxDir[MAX_PATH];
			WCHAR SandboxFile[MAX_PATH];
			InitSandboxDir(Session->boitSession->QQID,
				Session->AllocCompileID,
				ExecutableCodeFile,
				ExecutableFileName,
				SandboxDir, SandboxFile);

			if (Session->bNeedInput)
			{
				pINPUT_SESSION InputSession = malloc(sizeof(INPUT_SESSION));
				ZeroMemory(InputSession, sizeof(INPUT_SESSION));
				
				wcscpy_s(InputSession->Command, MAX_PATH, SandboxFile);
				wcscpy_s(InputSession->CuurentDir, MAX_PATH, SandboxDir);
				InputSession->bLimitPrivileges = !(Session->bIsSU);
				InputSession->Encode = Session->CompileCfg->OutputEncode;
				InputSession->boitSession = Session->boitSession;
				if (Session->boitSession->GroupID)
				{
					RegisterMessageWatch(BOIT_MW_GROUP_QQ, 10000000 * 20,
						Session->boitSession, InputCallback, (PBYTE)InputSession);
				}
				else
				{
					RegisterMessageWatch(BOIT_MW_QQ, 10000000 * 20,
						Session->boitSession, InputCallback, (PBYTE)InputSession);
				}
				SendBackMessage(Session->boitSession, L"请输入输入数据：");


			}
			else
			{
				if (!StartRunSandbox(NULL, SandboxFile, SandboxDir, !(Session->bIsSU), (Session->boitSession), Session->CompileCfg->OutputEncode, 0))
				{
					free(Session->boitSession);
				}
			}
			
		}
		else
		{
			if (Sandbox->ExitReason == SANDBOX_ER_TIMEOUT)
			{
				SendBackMessage(Session->boitSession, L"Oh... 编译超时了");
			}
			else
			{
				SendBackMessage(Session->boitSession, L"Oops... 编译失败了");
			}
			WCHAR* wcStdout;

			UINT CodePage = GetEncodeCodePage(Session->CompileCfg->OutputEncode);

			int cchStdout;
			wcStdout = StrConvMB2WC(CodePage, Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, &cchStdout);
			if (wcStdout)
			{
				if (cchStdout > BOIT_MAX_TEXTLEN)
				{
					wcStdout[BOIT_MAX_TEXTLEN] = 0;
				}

				WCHAR* ShowMessage = malloc(sizeof(WCHAR) * (cchStdout + 32));
				swprintf_s(ShowMessage, cchStdout + 32, L"编译器输出：\n%ls\n编译器返回值：%ld", wcStdout, CompileExitCode);
				free(wcStdout);
				SendBackMessage(Session->boitSession, ShowMessage);
				free(ShowMessage);
			}
			else
			{
				//TODO: 转换字符串 or 内存申请失败
			}

			free(Session->boitSession);
		}

		FreeSimpleSandbox(Sandbox);
		FreeVBuf(Session->StdOutBuffer);
		free(Session->CompileCfg);
		free(Session);
	}

	break;
	case SANDBOX_EVENT_STD_OUTPUT:
		if (Session->StdOutBuffer->Length + DataLen < BOIT_MAX_TEXTLEN * 4) // UTF8编码在最恶劣情况下一个字符4字节
		{
			int OrgDataLen = Session->StdOutBuffer->Length;
			AdjustVBuf(Session->StdOutBuffer, Session->StdOutBuffer->Length + DataLen);
			memcpy(Session->StdOutBuffer->Data + OrgDataLen, StdOutData, DataLen);
		}
		//剩下的直接丢掉包
		break;
	}

	return 0;
}


int RunSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen)
{
	pRUN_SESSION Session = (pRUN_SESSION)pData;
	switch (Event)
	{
	case SANDBOX_EVTNT_PROCESS_ZERO:
	{
		if (Sandbox->bMemoryExceed)
		{
			SendBackMessage(Session->boitSession, L"程序超出内存上限了qaq");
		}
		switch (Sandbox->ExitReason)
		{
		case SANDBOX_ER_ABNORMAL:
			SendBackMessage(Session->boitSession, L"程序异常终止了！qaq");
			break;
		case SANDBOX_ER_TIMEOUT:
			SendBackMessage(Session->boitSession, L"超出运行时间限制了！qaq");
			break;
		case SANDBOX_ER_KILLED:
			SendBackMessage(Session->boitSession, L"程序被强行中断了！qaq");
			break;
		}


		if (Session->StdOutBuffer->Length == 0)
		{
			SendBackMessage(Session->boitSession, L"程序莫得输出诶");
		}
		else
		{
			UINT CodePage = GetEncodeCodePage(Session->Encode);
			WCHAR* wcStdout;
			int cchStdout;
			
			wcStdout = StrConvMB2WC(CodePage, Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, &cchStdout);

			if (wcStdout)
			{
				//实行截断
				if (cchStdout > BOIT_RUN_MAX_OUTPUT)
				{
					wcStdout[BOIT_RUN_MAX_OUTPUT] = 0;
				}
				SendTextWithBOITCode(Session->boitSession, wcStdout);
				free(wcStdout);
			}
		
		}
		FreeBOITSession(Session->boitSession);
		FreeVBuf(Session->StdOutBuffer);
		free(Session);
	}
	break;
	case SANDBOX_EVENT_STD_OUTPUT:
	{
		if (Session->StdOutBuffer->Length + DataLen < BOIT_MAX_TEXTLEN * 4) // UTF8编码和GB编码在最恶劣情况下一个字符4字节
		{
			int OrgDataLen = Session->StdOutBuffer->Length;
			AdjustVBuf(Session->StdOutBuffer, Session->StdOutBuffer->Length + DataLen);
			memcpy(Session->StdOutBuffer->Data + OrgDataLen, StdOutData, DataLen);
		}
		//剩下的直接丢掉包
		break;
	}
	break;
	}

	return 0;
}


BOOL InitSandboxDir(LONGLONG QQID, LONGLONG AllocCompileID, WCHAR* ToCopyFile, WCHAR* ToCopyFileName, WCHAR* SandboxDir, WCHAR* SandboxFile)
{
	//WCHAR SandboxFile[MAX_PATH] = { 0 };
	WCHAR SandboxDirName[16];
	GetPerUserDir(SandboxFile, QQID);
	PathAppendW(SandboxFile, L"Sandbox\\");
	swprintf_s(SandboxDirName, _countof(SandboxDirName), L"Sandbox%lld", AllocCompileID);

	PathAppendW(SandboxFile, SandboxDirName);

	wcscpy_s(SandboxDir, MAX_PATH, SandboxFile);

	
	RemoveDirIfExist(SandboxDir);


	CreateDirectoryW(SandboxFile, 0);
	//我寻思上面这个目录变量也不用了，拿过来直接往后接可执行文件名吧
	PathAppendW(SandboxFile, ToCopyFileName);

	CopyFile(ToCopyFile, SandboxFile, TRUE);


	return 0;
}


pSANDBOX StartRunSandbox(WCHAR* Application, WCHAR* CommandLine, WCHAR* CuurentDir, BOOL bLimitPrivileges, pBOIT_SESSION boitSession, int Encode, WCHAR* Input)
{
	pSANDBOX Sandbox = 0;
	pRUN_SESSION RunSession = malloc(sizeof(RUN_SESSION));
	if (!RunSession)
	{
		return 0;
	}
	RunSession->Encode = Encode;

	RunSession->boitSession = boitSession;
	
	RunSession->StdOutBuffer = AllocVBuf();
	
	if ((Sandbox = CreateSimpleSandboxW(Application, CommandLine, CuurentDir,
		10000000 * 10,		//10秒
		512 * 1024 * 1024,	//512MB内存
		10 * 100,			//10% CPU
		bLimitPrivileges, (PBYTE)RunSession, RunSandboxCallback)) == 0)
	{
		SendBackMessage(RunSession->boitSession, L"为程序创建沙盒时出现意外");
		free(RunSession);
	}
	return Sandbox;
}


int CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_DIRINIT:
		//这里创建编译配置文件
		PerCommandCfgCreateDirIfNExist(pCmd, L"Compiler\\");
		//肯定没创建
		//创建说明文件
		break;
	case EC_CMDLOAD:
		pRunCmd = pCmd; // 存下来以备后用
		CompileID = 0;
		break;
	}
	return 0;
}


BOOL FindCompileConfig(pBOIT_COMMAND pCmd, WCHAR* LanguageName, int LanguageLen, WCHAR* ConfigSuffix, pCOMPILE_CFG CompileCfg)
{
	WCHAR CompilerCfgPath[MAX_PATH];
	GetPerCommandCfgDir(CompilerCfgPath, pCmd);
	PathAppendW(CompilerCfgPath, L"Compiler\\*");

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bNext = TRUE;

	BOOL bMatch = FALSE;
	for (hFind = FindFirstFileW(CompilerCfgPath, &FindData);
		hFind != INVALID_HANDLE_VALUE && wcslen(FindData.cFileName) > 0;
		bNext = FindNextFileW(hFind, &FindData))
	{
		if (!bNext)break;

		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (_wcsicmp(FindData.cFileName + wcslen(FindData.cFileName) - wcslen(ConfigSuffix), ConfigSuffix) == 0)
			{
				WCHAR CfgFilePath[MAX_PATH];
				GetPerCommandCfgDir(CfgFilePath, pCmd);
				PathAppendW(CfgFilePath, L"Compiler");
				PathAppendW(CfgFilePath, FindData.cFileName);
				if (MatchCompileConfig(CfgFilePath, CompileCfg, LanguageName, LanguageLen))
				{
					bMatch = TRUE;
					break;
				}
			}
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);


	return bMatch;
}


BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg, WCHAR* LanguageName, int LanguageLen)
{
	HANDLE hCfgFile = CreateFileW(ConfigFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCfgFile == INVALID_HANDLE_VALUE) return FALSE;
	PBYTE pData = 0;
	WCHAR* pwData = 0;
	BOOL bMatch = FALSE;

	__try
	{
		DWORD FileSize = GetFileSize(hCfgFile, 0);
		pData = malloc(FileSize + 1);
		if (!pData)__leave;
		ZeroMemory(pData, FileSize + 1);
		DWORD BytesRead;
		if (ReadFile(hCfgFile, pData, FileSize, &BytesRead, NULL) == FALSE)__leave;
		if (BytesRead != FileSize)
		{
			__leave;
		}

		int wLen;
		pwData = StrConvMB2WC(CP_ACP, pData, FileSize, &wLen);
		if (!pwData)
		{
			__leave;
		}

		//清空配置
		CompileCfg->Application[0] = 0;
		CompileCfg->Command[0] = 0;
		CompileCfg->OutputEncode = 0;
		CompileCfg->SourceEncode = 0;
		CompileCfg->SourceSuffix[0] = 0;
		CompileCfg->Name[0] = 0;
		CompileCfg->Type = COMPILE_TYPE_NULL;

		//解析文件
		WCHAR* pwParse = pwData;

		BOOL NameFound = 0, TypeFound = 0, SuffixFound = 0, CommandFound = 0;

		while (wLen > 0)
		{
			int LineFeedLen = GetLineFeedLen(pwParse);
			wLen -= LineFeedLen;
			pwParse += LineFeedLen;

			int LineLen = GetLineLen(pwParse);
			//解析行
			if (LineLen)
			{
				if (pwParse[0] != L'#')//注释
				{
					WCHAR FieldNameList[][16] = { L"Name",L"Type",L"Suffix",L"Command",L"SourceEncode",L"OutputEncode", L"Application" };

					WCHAR* LineParse = pwParse;
					for (int i = 0; i < _countof(FieldNameList); i++)
					{
						if (_wcsnicmp(FieldNameList[i], LineParse, wcslen(FieldNameList[i])) == 0)
						{
							LineParse += wcslen(FieldNameList[i]);
							LineParse += GetLineSpaceLen(LineParse);
							if (LineParse[0] != L'=')
							{
								break;
							}
							LineParse++;
							LineParse += GetLineSpaceLen(LineParse);
							switch (i)
							{
							case 0:
								//解析Name
							{
								int LangLen = GetCmdParamLen(LineParse);
								if ((!LangLen) || LangLen > COMPILE_MAXNAME)break;
								wcsncpy_s(CompileCfg->Name, COMPILE_MAXNAME, LineParse, LangLen);
							}
							if (LanguageName && LanguageLen)
							{
								while (1)
								{
									int LangLen = GetCmdParamLen(LineParse);
									if ((!LangLen) || LangLen > COMPILE_MAXNAME)break;

									if (LangLen == LanguageLen && (_wcsnicmp(LineParse, LanguageName, LangLen) == 0))
									{
										NameFound = TRUE; //匹配成功
									}
									LineParse += LangLen;
									LineParse += GetLineSpaceLen(LineParse);
								}
							}
							break;
							case 1:
								//解析Type
								if (_wcsnicmp(LineParse, L"Compile", wcslen(L"Compile")) == 0)
								{
									CompileCfg->Type = COMPILE_TYPE_COMPILE;
									TypeFound = TRUE;
								}
								else if (_wcsnicmp(LineParse, L"Script", wcslen(L"Script")) == 0)
								{
									CompileCfg->Type = COMPILE_TYPE_SCRIPT;
									TypeFound = TRUE;
								}
								break;
							case 2:
								//解析Suffix
							{
								int SuffixLen = GetCmdParamLen(LineParse);
								if (SuffixLen && SuffixLen < COMPILE_MAXSUFFIX)
								{
									wcsncpy_s(CompileCfg->SourceSuffix, COMPILE_MAXSUFFIX, LineParse, SuffixLen);
									SuffixFound = TRUE;
								}
							}
							break;
							case 3:
							{
								//解析Command
								int CommandLen = GetLineLen(LineParse);
								if (CommandLen && CommandLen < COMPILECMD_MAXLEN)
								{
									wcsncpy_s(CompileCfg->Command, COMPILECMD_MAXLEN, LineParse, CommandLen);
									CommandFound = TRUE;
								}
							}
							break;
							case 4:
							{
								if (_wcsnicmp(LineParse, L"GB18030", wcslen(L"GB18030")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_GB18030;
								}
								else if (_wcsnicmp(LineParse, L"UTF8", wcslen(L"UTF8")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_UTF8;
								}
								else if (_wcsnicmp(LineParse, L"ANSI", wcslen(L"ANSI")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_ANSI;
								}
								break;
							}
							break;
							case 5:
							{
								if (_wcsnicmp(LineParse, L"GB18030", wcslen(L"GB18030")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_GB18030;
								}
								else if (_wcsnicmp(LineParse, L"UTF8", wcslen(L"UTF8")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_UTF8;
								}
								else if (_wcsnicmp(LineParse, L"ANSI", wcslen(L"ANSI")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_ANSI;
								}
								break;
							}
							break;
							case 6:
							{
								//解析Command
								int ApplicationLen = GetLineLen(LineParse);
								if (ApplicationLen && ApplicationLen < MAX_PATH)
								{
									wcsncpy_s(CompileCfg->Application, MAX_PATH, LineParse, ApplicationLen);
								}
							}
							break;
							}

						}
					}


				}
			}

			pwParse += LineLen;
			wLen -= LineLen;
		}


		if (TypeFound && SuffixFound && CommandFound)
		{
			if (LanguageName)
			{
				if (NameFound)
				{
					bMatch = TRUE;
				}
			}
			else
			{
				if (CompileCfg->Name[0])
				{
					bMatch = TRUE;
				}
			}
		}

	}
	__finally
	{
		if (hCfgFile)
		{
			CloseHandle(hCfgFile);
		}
		if (pData)
		{
			free(pData);
		}
		if (pwData)
		{
			free(pwData);
		}
	}
	return bMatch;
}


BOOL ShowSupportLanguageInfo(pBOIT_COMMAND pCmd, WCHAR* ConfigSuffix, pBOIT_SESSION boitSession)
{
	COMPILE_CFG CompileCfg;
	WCHAR CompilerCfgPath[MAX_PATH];
	GetPerCommandCfgDir(CompilerCfgPath, pCmd);
	PathAppendW(CompilerCfgPath, L"Compiler\\*");

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bNext = TRUE;

	BOOL bMatch = FALSE;

	WCHAR ReplyMessage[256] = L"支持语言如下：\n";

	for (hFind = FindFirstFileW(CompilerCfgPath, &FindData);
		hFind != INVALID_HANDLE_VALUE && wcslen(FindData.cFileName) > 0;
		bNext = FindNextFileW(hFind, &FindData))
	{
		if (!bNext)break;

		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (_wcsicmp(FindData.cFileName + wcslen(FindData.cFileName) - wcslen(ConfigSuffix), ConfigSuffix) == 0)
			{
				WCHAR CfgFilePath[MAX_PATH];
				GetPerCommandCfgDir(CfgFilePath, pCmd);
				PathAppendW(CfgFilePath, L"Compiler");
				PathAppendW(CfgFilePath, FindData.cFileName);
				if (MatchCompileConfig(CfgFilePath, &CompileCfg, 0, 0))
				{
					wcscat_s(ReplyMessage, _countof(ReplyMessage), CompileCfg.Name);
					wcscat_s(ReplyMessage, _countof(ReplyMessage), L"  ");

				}
			}
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);

	SendBackMessage(boitSession, ReplyMessage);
	return 0;
}


int GetLineLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == 0)
		{
			break;
		}
	}
	return i;
}


int GetLineSpaceLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\t')
		{
			break;
		}
	}
	return i;
}


int GetLineFeedLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\r' &&
			String[i] != L'\n')
		{
			break;
		}
	}
	return i;
}


BOOL GetCompileCommand(WCHAR* CommandBuffer, pCOMPILE_CFG CompileCfg, LONGLONG AllocCompileID)
{
	int len = wcslen(CompileCfg->Command);
	WCHAR SourceFile[16];
	WCHAR ExecutableFile[16];

	swprintf_s(SourceFile, _countof(SourceFile), L"Temp%lld%ls", AllocCompileID, CompileCfg->SourceSuffix);
	swprintf_s(ExecutableFile, _countof(ExecutableFile), L"Temp%lld.exe", AllocCompileID);
	int j = 0;
	for (int i = 0; i < len; )
	{
		if (j >= COMPILECMD_MAXLEN - 1)
		{
			break;
		}
		if (_wcsnicmp(CompileCfg->Command + i, L"%In", wcslen(L"%In")) == 0)
		{
			wcscpy_s(CommandBuffer + j, COMPILECMD_MAXLEN - j, SourceFile);
			i += 3;
			j += wcslen(SourceFile);
		}
		else if (_wcsnicmp(CompileCfg->Command + i, L"%Out", wcslen(L"%Out")) == 0)
		{
			wcscpy_s(CommandBuffer + j, COMPILECMD_MAXLEN - j, ExecutableFile);
			i += 4;
			j += wcslen(ExecutableFile);
		}
		else if (_wcsnicmp(CompileCfg->Command + i, L"%%", wcslen(L"%%")) == 0)
		{
			CommandBuffer[j++] = CompileCfg->Command[i++];
			i++;
		}
		else
		{
			CommandBuffer[j++] = CompileCfg->Command[i++];
		}
	}
	CommandBuffer[j++] = 0;
	return 0;
}


UINT GetEncodeCodePage(int Compile_Encode)
{
	UINT CodePage = 0;
	switch (Compile_Encode)
	{
	case COMPILE_ENCODE_UTF8:
		CodePage = CP_UTF8;
		break;

	case COMPILE_ENCODE_ANSI:
		CodePage = CP_ACP;
		break;

	case COMPILE_ENCODE_GB18030:
	default:
		CodePage = CP_GB18030; //详见  https://docs.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers
		break;
	}
	return CodePage;
}



int InputCallback(long long MsgWatchID, PBYTE pData, UINT Event,
	long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	pINPUT_SESSION InputSession = (pINPUT_SESSION)pData;
	int iRet = BOIT_MSGWATCH_PASS;
	switch (Event)
	{
	case BOIT_MW_EVENT_TIMEOUT:
		SendBackMessage(InputSession->boitSession, Corpus_WhereIsInput());
		free(InputSession->boitSession);
		break;
	case BOIT_MW_EVENT_MESSAGE:
	{
		pSANDBOX Sandbox = StartRunSandbox(InputSession->Application[0] ? InputSession->Application : NULL,
			InputSession->Command, InputSession->CuurentDir, InputSession->bLimitPrivileges, InputSession->boitSession, InputSession->Encode, 0);

		
		if (Sandbox)
		{
			UINT CodePage = GetEncodeCodePage(InputSession->Encode);
			PBYTE mbStr;
			

			int mbStrlen;
			mbStr = StrConvWC2MB(CodePage, Msg, -1, &mbStrlen);
			if(mbStr)
			{
				DWORD BytesWritten;
				WriteFile(Sandbox->hPipeInWrite, mbStr, mbStrlen, &BytesWritten, 0);
				CloseHandle(Sandbox->hPipeInWrite);
				Sandbox->hPipeInWrite = 0;
				free(mbStr);
			}
			else
			{
				//StrConvWC2MB失败
			}

			
			iRet = BOIT_MSGWATCH_BLOCK;
		}
		else
		{
			free(InputSession->boitSession);
		}
	}
	break;
	}
	RemoveMessageWatchByID(MsgWatchID);
	free(InputSession);

	return iRet;
}

