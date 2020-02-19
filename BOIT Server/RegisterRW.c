#include<Windows.h>
#include"Settings.h"
#include<strsafe.h>
#include "RegisterRW.h"

#define REG_STORE_PLACE HKEY_CURRENT_USER

#define LeaveIfRegFailed(Result, iSuccess) \
if(Result != ERROR_SUCCESS)\
{\
	iSuccess = SETTINGS_ERROR;\
	__leave;\
}

int RegisterRead(WCHAR * BaseDir)
{
	HKEY RootKey = 0;
	LONG Result = ERROR_SUCCESS;
	DWORD cbDataSize = 0;
	int iSuccess = SETTINGS_LOADED;
	__try
	{
		Result = RegOpenKeyEx(REG_STORE_PLACE, TEXT("Software\\BOIT"), 0, KEY_QUERY_VALUE, &RootKey);
		switch (Result)
		{
		case ERROR_SUCCESS:
			break;
		case ERROR_FILE_NOT_FOUND:
			iSuccess = SETTINGS_NOT_FOUND;
			__leave;
		default:
			iSuccess = SETTINGS_ERROR;
			__leave;
		}

		DWORD VersionMajor, VersionMinor;

		cbDataSize = sizeof(DWORD);
		DWORD RegType = REG_DWORD;
		Result = RegQueryValueEx(RootKey, TEXT("VersionMajor"), 0, 0, (LPBYTE)&VersionMajor, &cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		cbDataSize = sizeof(DWORD);
		Result = RegQueryValueEx(RootKey, TEXT("VersionMinor"), 0, 0, (LPBYTE)&VersionMinor, &cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		if (VersionMajor < VERSION_MAJOR)
		{
			//老版本配置
			iSuccess = SETTINGS_OUT_OF_DATE;
			__leave;
		}
		if (VersionMajor > VERSION_MAJOR)
		{
			//老版本软件
			iSuccess = SETTINGS_ERROR;
			__leave;
		}

		if (VersionMinor < VERSION_MINOR)
		{
			//老版本配置
			iSuccess = SETTINGS_OUT_OF_DATE;
			__leave;
		}
		if (VersionMinor > VERSION_MINOR)
		{
			//老版本软件
			iSuccess = SETTINGS_ERROR;
			__leave;
		}

		cbDataSize = sizeof(WCHAR) * MAX_PATH;
		Result = RegQueryValueEx(RootKey, TEXT("BaseDir"), 0, 0, (LPBYTE)BaseDir, &cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);
	}
	__finally
	{
		if (RootKey)
		{
			RegCloseKey(RootKey);
		}
	}
	return iSuccess;
}

int RegisterWrite(WCHAR* BaseDir)
{
	HKEY RootKey = 0;
	HKEY hSectionKey = 0;
	HKEY hItemKey = 0;
	LONG Result;
	DWORD cbDataSize = 0;
	int iSuccess = SETTINGS_SAVED;

	__try
	{
		Result = RegOpenKeyEx(REG_STORE_PLACE, TEXT("Software\\BOIT"), 0, KEY_QUERY_VALUE, &RootKey);
		LeaveIfRegFailed(Result, iSuccess);

		DWORD VersionMajor = VERSION_MAJOR, VersionMinor = VERSION_MINOR;

		cbDataSize = sizeof(DWORD);
		DWORD RegType = REG_DWORD;
		Result = RegSetValueEx(RootKey, TEXT("VersionMajor"), 0, RegType, (const BYTE*)&VersionMajor, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		cbDataSize = sizeof(DWORD);
		Result = RegSetValueEx(RootKey, TEXT("VersionMinor"), 0, RegType, (const BYTE*)&VersionMinor, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		cbDataSize = sizeof(WCHAR) * MAX_PATH;
		Result = RegSetValueEx(RootKey, TEXT("BaseDir"), 0, 0, (const BYTE*)BaseDir, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

	}
	__finally
	{
		if (RootKey)
		{
			RegCloseKey(RootKey);
		}
	}
	return iSuccess;
}

int InitializeSettings(WCHAR* BaseDir)
{
	HKEY RootKey = 0;
	LONG Result;
	DWORD cbDataSize = 0;
	int iSuccess = SETTINGS_INITIALIZED;
	__try
	{
		Result = RegCreateKeyEx(REG_STORE_PLACE, TEXT("Software\\BOIT"), 0, NULL, (DWORD)NULL, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY, 0, &RootKey, NULL);
		LeaveIfRegFailed(Result, iSuccess);

		DWORD VersionMajor = VERSION_MAJOR, VersionMinor = VERSION_MINOR;

		cbDataSize = sizeof(DWORD);
		DWORD RegType = REG_DWORD;
		Result = RegSetValueEx(RootKey, TEXT("VersionMajor"), 0, RegType, (const BYTE*)&VersionMajor, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		cbDataSize = sizeof(DWORD);
		Result = RegSetValueEx(RootKey, TEXT("VersionMinor"), 0, RegType, (const BYTE*)&VersionMinor, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);

		cbDataSize = sizeof(WCHAR) * MAX_PATH;
		RegType = REG_SZ;
		Result = RegSetValueEx(RootKey, TEXT("BaseDir"), 0, RegType, (const BYTE*)BaseDir, cbDataSize);
		LeaveIfRegFailed(Result, iSuccess);
	}
	__finally
	{
		//执行清理
		if (RootKey)
		{
			RegCloseKey(RootKey);
		}

	}
	return iSuccess;
}


int ClearSettings()
{
	//清除所有设置

	HKEY BaseKey = 0;
	HKEY RootKey = 0;
	LONG Result;
	int iSuccess = SETTINGS_CLEARED;

	__try
	{
		Result = RegOpenKeyEx(REG_STORE_PLACE, TEXT("Software"), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | DELETE, &BaseKey);
		LeaveIfRegFailed(Result, iSuccess);


		Result = RegDeleteTree(BaseKey, TEXT("BOIT"));
		LeaveIfRegFailed(Result, iSuccess);
	}
	__finally
	{
		if (BaseKey) 
		{
			RegCloseKey(BaseKey);
		}
	}
	return iSuccess;
}
