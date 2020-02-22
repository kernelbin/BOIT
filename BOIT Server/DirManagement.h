#pragma once
#include<Windows.h>
#include<Shlwapi.h>
#include"CommandManager.h"

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

BOOL RemoveDirIfExist(WCHAR* Dir);

int CheckPerUserDataExist(long long QQID);

int CreatePerUserData(WCHAR* Path);

int GetPerUserDir(WCHAR* Buffer, long long QQID);

BOOL PerUserCreateDirIfNExist(long long QQID, WCHAR* FolderName);

BOOL PerUserCreateDirIfNExist(long long QQID, WCHAR* FolderName);

int GetPerCommandCfgDir(WCHAR* Buffer, pBOIT_COMMAND pCmd);

BOOL PerCommandCfgCreateDirIfNExist(pBOIT_COMMAND pCmd, WCHAR* FolderName);


//权限相关目录操作
BOOL CheckUserToken(long long QQID, WCHAR* TokenStr);