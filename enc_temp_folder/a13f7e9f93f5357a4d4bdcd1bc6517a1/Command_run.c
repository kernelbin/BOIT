#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"DirManagement.h"
#include<wchar.h>

#define COMPILECMD_MAXLEN 512

#define COMPILE_MAXSUFFIX 16


#define COMPILE_TYPE_NULL 0
#define	COMPILE_TYPE_COMPILE 1
#define COMPILE_TYPE_SCRIPT 2
typedef struct __tagCompileCfg
{
	int Type;
	WCHAR SourceSuffix[COMPILE_MAXSUFFIX];
	WCHAR Command[COMPILECMD_MAXLEN];
}COMPILE_CFG, * pCOMPILE_CFG;


BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg, WCHAR* LanguageName, int LanguageLen);

int GetLineLen(WCHAR* String);

int GetLineFeedLen(WCHAR* String);

int GetLineSpaceLen(WCHAR* String);


MSGPROC CmdMsg_run_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	//检查用户目录下是否有相应文件夹
	if (PerUserCreateDirIfNExist(QQID, L"Sandbox"))
	{
		//创建相应文件
	}
	if (PerUserCreateDirIfNExist(QQID, L"Compile"))
	{
		//创建相应文件
	}

	//TODO:在这里读取用户信息，是否有权限执行程序，是否有权限以su执行程序


		//usage: #run language [/su] sourcecode
	int MsgLen = wcslen(Msg);
	WCHAR* lpwcParam = Msg;
	int ParamCnt = 0;
	COMPILE_CFG CompileCfg;

	BOOL LanguageMatched = 0, bCodeFound = 0;
	WCHAR* CodeStr = 0;

	BOOL bFailed = 0, bIsSU = 0;

	WCHAR FailedReason[128];
	while (MsgLen > 0)
	{
		int ParamLen = GetCmdParamLen(lpwcParam);
		//处理这个 Param
		if (ParamCnt == 1)
		{
			if (FindCompileConfig(pCmd, lpwcParam, ParamLen, L".cfg", &CompileCfg) == TRUE)
			{
				LanguageMatched = TRUE;
			}
		}
		else if (ParamCnt > 1)
		{
			//解析可能的参数
			if (lpwcParam[0] == '-' || lpwcParam[0] == '/' || lpwcParam[0] == '\\')
			{
				int ParamLen = GetCmdParamLen(lpwcParam);
				if (ParamLen == 1)
				{
					swprintf_s(FailedReason, _countof(FailedReason), L"无法解析参数 %lc", lpwcParam[0]);
					SendBackMessage(GroupID, QQID, FailedReason);
					bFailed = 1;
					break;
				}
				else if (ParamLen >= 16)
				{
					SendBackMessage(GroupID, QQID, L"参数名称过长");
					bFailed = 1;
					break;
				}
				else
				{
					if ((ParamLen == wcslen(L"su") + 1) && _wcsnicmp(lpwcParam + 1, L"su", wcslen(L"su")) == 0)
					{
						bIsSU = TRUE;
					}
					else
					{
						WCHAR ParamBuf[16] = { 0 };
						wcsncpy_s(ParamBuf, _countof(ParamBuf), lpwcParam, ParamLen);
						swprintf_s(FailedReason, _countof(FailedReason), L"无法解析参数 %ls", ParamBuf);
						SendBackMessage(GroupID, QQID, FailedReason);
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
		ParamCnt++;
	}

	if (!bFailed)
	{
		if (LanguageMatched)
		{
			if (bCodeFound)
			{
				//撒花！
				SendBackMessage(GroupID, QQID, CodeStr);
			}
			else
			{
				SendBackMessage(GroupID, QQID, L"未找到源代码");
				bFailed = TRUE;
			}
		}
		else
		{
			SendBackMessage(GroupID, QQID, L"未找到语言类型或该语言不受支持");
			bFailed = TRUE;
		}
	}
	

	if (bFailed)
	{
		SendBackMessage(GroupID, QQID, L"usage: #run language [/su] sourcecode");
	}



	//SendBackMessage(GroupID, QQID, GetCmdParamLen(Msg) + Msg);
	return 0;
}

EVENTPROC CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_DIRINIT:
		//这里创建编译配置文件
		PerCommandCfgCreateDirIfNExist(pCmd, L"Compiler\\");
		//肯定没创建
		//创建说明文件
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
	HFILE hCfgFile = CreateFileW(ConfigFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCfgFile == INVALID_HANDLE_VALUE) return FALSE;
	PBYTE pData = 0;
	WCHAR* pwData = 0;
	BOOL bMatch = FALSE;

	__try
	{
		DWORD FileSize = GetFileSize(hCfgFile, 0);
		pData = malloc(FileSize + 1);
		ZeroMemory(pData, FileSize + 1);
		DWORD BytesRead;
		ReadFile(hCfgFile, pData, FileSize, &BytesRead, NULL);
		if (BytesRead != FileSize)
		{
			__leave;
		}

		int wLen = MultiByteToWideChar(CP_ACP, 0, pData, FileSize, 0, 0);

		pwData = (WCHAR*)malloc(sizeof(WCHAR) * (wLen + 1));
		ZeroMemory(pwData, sizeof(WCHAR) * (wLen + 1));
		MultiByteToWideChar(CP_ACP, 0, pData, FileSize, pwData, wLen);


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
					WCHAR FieldNameList[][16] = { L"Name",L"Type",L"Suffix",L"Command" };

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
								while (1)
								{
									int LangLen = GetCmdParamLen(LineParse);
									if (!LangLen)break;

									if (LangLen == LanguageLen && (_wcsnicmp(LineParse, LanguageName, LangLen) == 0))
									{
										NameFound = TRUE; //匹配成功
									}
									LineParse += LangLen;
									LineParse += GetLineSpaceLen(LineParse);
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
								int CommandLen = GetLineLen(LineParse);
								if (CommandLen && CommandLen < COMPILECMD_MAXLEN)
								{
									wcsncpy_s(CompileCfg->Command, COMPILECMD_MAXLEN, LineParse, CommandLen);
									CommandFound = TRUE;
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


		if (NameFound && TypeFound && SuffixFound && CommandFound)
		{
			bMatch = TRUE;
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