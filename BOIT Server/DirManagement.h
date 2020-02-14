#pragma once
#include<Windows.h>
#include<Shlwapi.h>

WCHAR* GetBOITBaseDir();

int InitBOITDirVar();

int InitBOITDir();

WCHAR* GetBOITUserDataDir();

WCHAR* GetBOITCommandCfgDir();

WCHAR BOITBaseDir[MAX_PATH];
WCHAR BOITUserDataDir[MAX_PATH];
WCHAR BOITCommandCfgDir[MAX_PATH];

BOOL IsPathDirA(CHAR* Path);
BOOL IsPathDirW(WCHAR* Path);

int PathSimplifyA(CHAR* Path);
int PathSimplifyW(WCHAR* Path);


int CheckPerUserDataExist(long long QQID);

int CreatePerUserData(WCHAR* Path);
