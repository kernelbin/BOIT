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

	wcscpy_s(BOITCommandCfgDir, MAX_PATH, BOITBaseDir);
	PathAppendW(BOITCommandCfgDir, L"CommandCfg\\");
	return 0;
}


int InitBOITDir()
{
	//创建各类路径
	CreateDirectoryW(GetBOITBaseDir(), 0);
	CreateDirectoryW(GetBOITUserDataDir(), 0);
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

