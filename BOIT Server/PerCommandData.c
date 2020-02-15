#include<Windows.h>
#include"DirManagement.h"
#include<wchar.h>
#include"CommandManager.h"

int GetPerCommandCfgDir(WCHAR* Buffer, pBOIT_COMMAND pCmd)
{
	wcscpy_s(Buffer, MAX_PATH, GetBOITCommandCfgDir());
	PathAppendW(Buffer, pCmd->CommandName[0]);
	return 0;
}


BOOL PerCommandCfgCreateDirIfNExist(pBOIT_COMMAND pCmd, WCHAR * FolderName) //返回 TRUE 说明创建了，返回 FALSE 代表本来就有
{
	WCHAR DirPath[MAX_PATH];
	GetPerCommandCfgDir(DirPath, pCmd);
	PathAppendW(DirPath, FolderName);
	if (PathIsDirectoryW(DirPath) == 0)
	{
		CreateDirectoryW(DirPath, 0);
		return TRUE;
	}
	return FALSE;
}
