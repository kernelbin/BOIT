#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"DirManagement.h"

#define COMPILECMD_MAXLEN 512

#define COMPILE_TYPE_NULL 0
#define	COMPILE_TYPE_COMPILE 1
#define CPMPILE_TYPE_SCRPIT 2
typedef struct __tagCompileCfg
{
	int Type;
	WCHAR SourceSuffix[16];
	WCHAR Command[COMPILECMD_MAXLEN];
}COMPILE_CFG, * pCOMPILE_CFG;


BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg);

int GetLineLen(WCHAR* String);

int GetLineFeedLen(WCHAR* String);


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
	/*这里！解析输入数据，找出语言
		从command的配置文件里枚举文件夹，枚举文件夹里的编译器配置
	如果匹配到了，那么就分配一个编译ID，（加一个新的CmdEvent，指令初始化，初始分配ID和线程池）
		在Compile文件夹里新建一个文件夹把源代码塞进去编译
		编译完丢进sandbox
		沙盒运行*/


		//usage: #run language [/su] sourcecode
	int MsgLen = wcslen(Msg);
	WCHAR* lpwcParam = Msg;
	int ParamCnt = 0;
	COMPILE_CFG CompileCfg;
	while (MsgLen > 0)
	{
		int ParamLen = GetParamLen(lpwcParam);
		//处理这个 Param
		if (ParamCnt == 1)
		{
			SendBackMessage(GroupID, QQID, L"语言");
			SendBackMessage(GroupID, QQID, lpwcParam);
			FindCompileConfig(pCmd, lpwcParam, ParamLen, L".cfg", &CompileCfg);
		}

		lpwcParam += ParamLen;
		MsgLen -= ParamLen;
		int SpaceLen = GetSpaceLen(lpwcParam);
		lpwcParam += SpaceLen;
		MsgLen -= SpaceLen;
		ParamCnt++;
	}
	//SendBackMessage(GroupID, QQID, GetParamLen(Msg) + Msg);
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
				MatchCompileConfig(CfgFilePath, CompileCfg);
			}
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);


	return 0;
}



BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg)
{
	HFILE hCfgFile = CreateFileW(ConfigFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCfgFile == INVALID_HANDLE_VALUE) return FALSE;
	PBYTE pData = 0;
	WCHAR * pwData = 0;
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
		
		pwData = (WCHAR *)malloc(sizeof(WCHAR) * (wLen + 1));
		ZeroMemory(pwData, sizeof(WCHAR) * (wLen + 1));
		MultiByteToWideChar(CP_ACP, 0, pData, FileSize, pwData, wLen);
		

		//解析文件
		WCHAR* pwParse = pwData;
		while (wLen>0)
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
					MessageBoxW(0, pwParse, L"qwq", 0);
				}
			}
			
			pwParse += LineLen;
			wLen -= LineLen;
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