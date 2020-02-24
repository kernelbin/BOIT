#include<Windows.h>
#include"DirManagement.h"
#include <Shlwapi.h>
#include"CommandManager.h"

#pragma comment(lib, "Shlwapi.lib")


WCHAR * GetBOITBaseDir()
{
	return BOITBaseDir;
}


int InitBOITDirVar()//初始化各种目录路径
{
	
	wcscpy_s(BOITUserDataDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITUserDataDir, L"UserData\\");

	wcscpy_s(BOITGroupDataDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITGroupDataDir, L"GroupData\\");

	wcscpy_s(BOITCommandCfgDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITCommandCfgDir, L"CommandCfg\\");
	return 0;
}


int InitBOITDir()
{
	//创建各类路径
	CreateDirectoryW(GetBOITBaseDir(), 0);
	CreateDirectoryW(GetBOITUserDataDir(), 0);
	CreateDirectoryW(GetBOITGroupDataDir(), 0);
	CreateDirectoryW(GetBOITCommandCfgDir(), 0);

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

		FileOp.fFlags = FOF_NOCONFIRMATION;
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