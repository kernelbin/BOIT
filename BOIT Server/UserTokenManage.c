#include<Windows.h>
#include"DirManagement.h"

BOOL CheckUserToken(long long QQID, WCHAR* TokenStr)
{
	if (PerUserCreateDirIfNExist(QQID, L"Token"))
	{
		//初始化默认权限
	}

	WCHAR TokenFile[MAX_PATH];
	GetPerUserDir(TokenFile, QQID);
	PathAppendW(TokenFile, L"Token");
	PathAppendW(TokenFile, TokenStr);
	if (PathFileExistsW(TokenFile))
	{
		//TODO: 校验组权限
		return TRUE;
	}

	return FALSE;
}

