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

