#include<Windows.h>
#include"DirManagement.h"
#include <Shlwapi.h>
#include"CommandManager.h"
#include<wchar.h>


#pragma comment(lib, "Shlwapi.lib")


static long long CoolQPicID = 0;


WCHAR * GetBOITBaseDir()
{
	return BOITBaseDir;
}


WCHAR* GetCQBaseDir()
{
	return CQBaseDir;
}

int InitCQDirVar()
{
	wcscpy_s(CQIamgeDir, MAX_PATH, CQBaseDir);
	PathAppendW(CQIamgeDir, L"data\\image\\");
	return 0;
}

WCHAR* GetCQImageDir()
{
	return CQIamgeDir;
}


int InitBOITDirVar()//初始化各种目录路径
{
	
	wcscpy_s(BOITUserDataDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITUserDataDir, L"UserData\\");

	wcscpy_s(BOITGroupDataDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITGroupDataDir, L"GroupData\\");

	wcscpy_s(BOITCommandCfgDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITCommandCfgDir, L"CommandCfg\\");

	wcscpy_s(BOITMsgReplyDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITMsgReplyDir, L"MsgReply\\");
	return 0;
}


int InitBOITDir()
{
	//创建各类路径
	CreateDirectoryW(GetBOITBaseDir(), 0);
	CreateDirectoryW(GetBOITUserDataDir(), 0);
	CreateDirectoryW(GetBOITGroupDataDir(), 0);
	CreateDirectoryW(GetBOITCommandCfgDir(), 0);
	CreateDirectoryW(GetBOITMsgReplyDir(), 0);


	AcquireSRWLockShared(&CommandChainLock);
	WCHAR PerCommandDir[MAX_PATH];
	
	if (RootCommand)
	{
		for (pBOIT_COMMAND pList = RootCommand; pList; pList = pList->NextCommand)
		{
			wcscpy_s(PerCommandDir, MAX_PATH, GetBOITCommandCfgDir());
			PathAppendW(PerCommandDir, pList->CommandName[0]);
			CreateDirectoryW(PerCommandDir, 0);

			SendCommandEvent(pList, EC_DIRINIT, 0, 0);
		}
	}
	ReleaseSRWLockShared(&CommandChainLock);
	return 0;
}

WCHAR* GetBOITUserDataDir()
{
	return BOITUserDataDir;
}
WCHAR* GetBOITGroupDataDir()
{
	return BOITGroupDataDir;
}
WCHAR* GetBOITCommandCfgDir()
{
	return BOITCommandCfgDir;
}
WCHAR* GetBOITMsgReplyDir()
{
	return BOITMsgReplyDir;
}


BOOL IsPathDirA(CHAR * Path)
{
	if (PathIsDirectoryA(Path) == FILE_ATTRIBUTE_DIRECTORY)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL IsPathDirW(WCHAR* Path)
{
	if (PathIsDirectoryW(Path) == FILE_ATTRIBUTE_DIRECTORY)
	{
		return TRUE;
	}
	return FALSE;
}

int PathSimplifyA(CHAR* Path)
{
	char Buffer[MAX_PATH];
	PathCanonicalizeA(Buffer, Path);
	strcpy_s(Path,MAX_PATH, Buffer);
	return 0;
}

int PathSimplifyW(WCHAR* Path)
{
	WCHAR Buffer[MAX_PATH];
	PathCanonicalizeW(Buffer, Path);
	wcscpy_s(Path, MAX_PATH, Buffer);
	return 0;
}



BOOL RemoveDirIfExist(WCHAR * Dir)
{
	WCHAR* Buffer = 0;
	int iRet;
	if (PathIsDirectoryW(Dir))
	{
		SHFILEOPSTRUCTW FileOp;
		int Len = wcslen(Dir);
		Buffer = malloc(sizeof(WCHAR) * (Len + 2));
		wcscpy_s(Buffer, (Len + 2), Dir);
		Buffer[Len] = 0;
		Buffer[Len + 1] = 0;

		FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NO_UI;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = Buffer;
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;
		iRet = SHFileOperationW(&FileOp);
	}
	else 
	{
		return FALSE;
	}
	if(Buffer)free(Buffer);

	if (iRet == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	//TODO:
}


BOOL CoolQAllocPicFileName(WCHAR * FileName)
{
	long long AllocID = InterlockedIncrement64(&CoolQPicID);
	swprintf_s(FileName, MAX_PATH, L"Pic%lld", AllocID);
	return TRUE;
}