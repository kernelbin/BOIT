#pragma once
#include<Windows.h>
#include<Shlwapi.h>
#include"CommandManager.h"

WCHAR* GetBOITBaseDir();

int InitBOITDirVar();

int InitBOITDir();

WCHAR* GetBOITUserDataDir();

WCHAR* GetBOITGroupDataDir();

WCHAR* GetBOITCommandCfgDir();

WCHAR BOITBaseDir[MAX_PATH];
WCHAR BOITUserDataDir[MAX_PATH];
WCHAR BOITGroupDataDir[MAX_PATH];
WCHAR BOITCommandCfgDir[MAX_PATH];

BOOL IsPathDirA(CHAR* Path);
BOOL IsPathDirW(WCHAR* Path);

int PathSimplifyA(CHAR* Path);
int PathSimplifyW(WCHAR* Path);

BOOL RemoveDirIfExist(WCHAR* Dir);

int CheckPerUserDataExist(long long QQID);

int CreatePerUserData(WCHAR* Path);

int GetPerUserDir(WCHAR* Buffer, long long QQID);

int GetPerUserStorageDir(WCHAR* Buffer, long long QQID);

BOOL PerUserCreateDirIfNExist(long long QQID, WCHAR* FolderName);

HANDLE PerUserCreateStorageFile(long long QQID, WCHAR* FileName, DWORD DesiredAccess, DWORD dwShareMode, DWORD CreationDisposition);

int CheckPerGroupDataExist(long long GroupID);

int CreatePerGroupData(WCHAR* Path);

int GetPerGroupDir(WCHAR* Buffer, long long GroupID);

BOOL PerGroupCreateDirIfNExist(long long GroupID, WCHAR* FolderName);

BOOL PerGroupCreateFileIfNExist(long long GroupID, WCHAR* FileName);

int GetPerCommandCfgDir(WCHAR* Buffer, pBOIT_COMMAND pCmd);

BOOL PerCommandCfgCreateDirIfNExist(pBOIT_COMMAND pCmd, WCHAR* FolderName);


//权限相关目录操作
BOOL CheckUserToken(long long QQID, WCHAR* TokenStr);

BOOL CheckGroupToken(long long GroupID, WCHAR* TokenStr);
