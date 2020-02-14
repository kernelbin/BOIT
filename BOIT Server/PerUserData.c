#include<Windows.h>
#include"DirManagement.h"
#include<wchar.h>

int CheckPerUserDataExist(long long QQID)
{
	WCHAR PerUserDir[MAX_PATH];
	wcscpy_s(PerUserDir, MAX_PATH, GetBOITUserDataDir());
	WCHAR QQIDStr[16] = { 0 };
	swprintf_s(QQIDStr, 16, L"%lld\\", QQID);

	PathAppendW(PerUserDir, QQIDStr);

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

	WCHAR FolderPath[MAX_PATH];
	wcscpy_s(FolderPath, MAX_PATH, Path);
	PathAppendW(FolderPath, L"Sandbox\\"); //沙盒环境

	CreateDirectoryW(FolderPath, 0);

	wcscpy_s(FolderPath, MAX_PATH, Path);
	PathAppendW(FolderPath, L"Compile\\"); //编译环境

	CreateDirectoryW(FolderPath, 0);
	return 0;
}

