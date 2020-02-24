#include<Windows.h>
#include"DirManagement.h"
#include<wchar.h>

int CheckPerUserDataExist(long long QQID)
{
	WCHAR PerUserDir[MAX_PATH];
	GetPerUserDir(PerUserDir, QQID);

	if (PathFileExistsW(PerUserDir) == 0)
	{
		//创建用户目录
		CreatePerUserData(PerUserDir);
	}
	return 0;
}

int CreatePerUserData(WCHAR * Path)
{
	CreateDirectoryW(Path,0);//创建文件夹
	
	WCHAR DirPath[MAX_PATH] = { 0 };
	//创建个人物品储存柜

	wcscpy_s(DirPath, MAX_PATH, Path);
	PathAppendW(DirPath, L"Storage");

	CreateDirectoryW(DirPath, 0);
	return 0;
}


int GetPerUserDir(WCHAR * Buffer,long long QQID)
{
	wcscpy_s(Buffer, MAX_PATH, GetBOITUserDataDir());
	WCHAR QQIDStr[16] = { 0 };
	swprintf_s(QQIDStr, 16, L"%lld\\", QQID);
	PathAppendW(Buffer, QQIDStr);
	return 0;
}

int GetPerUserStorageDir(WCHAR* Buffer, long long QQID)
{
	wcscpy_s(Buffer, MAX_PATH, GetBOITUserDataDir());
	WCHAR QQIDStr[16] = { 0 };
	swprintf_s(QQIDStr, 16, L"%lld", QQID);
	PathAppendW(Buffer, QQIDStr);
	PathAppendW(Buffer, L"Storage");
	return 0;
}



BOOL PerUserCreateDirIfNExist(long long QQID, WCHAR* FolderName) //返回 TRUE 说明创建了，返回 FALSE 代表本来就有
{
	WCHAR DirPath[MAX_PATH];
	GetPerUserDir(DirPath, QQID);
	PathAppendW(DirPath, FolderName);
	if (PathIsDirectoryW(DirPath) == 0)
	{
		CreateDirectoryW(DirPath, 0);
		return TRUE;
	}
	return FALSE;
}


BOOL PerUserCreateFileIfNExist(long long QQID, WCHAR* FileName) //返回 TRUE 说明创建了，返回 FALSE 代表本来就有
{
	WCHAR FilePath[MAX_PATH];
	GetPerUserDir(FilePath, QQID);
	PathAppendW(FilePath, FileName);
	if (PathFileExistsW(FilePath) == 0)
	{
		HANDLE hFile = CreateFile(FilePath, GENERIC_READ, 0, 0, CREATE_NEW, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}



HANDLE PerUserCreateStorageFile(long long QQID, WCHAR* FileName,DWORD DesiredAccess,DWORD dwShareMode,DWORD CreationDisposition) //返回 TRUE 说明创建了，返回 FALSE 代表本来就有
{
	WCHAR FilePath[MAX_PATH];
	GetPerUserStorageDir(FilePath, QQID);
	PathAppendW(FilePath, FileName);

	HANDLE hFile = CreateFile(FilePath, DesiredAccess, dwShareMode, 0, CreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);

	return hFile;
}
