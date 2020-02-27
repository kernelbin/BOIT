#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"DirManagement.h"


int RunCode(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);


int CmdMsg_runcode_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	HANDLE hSavedFile = PerUserCreateStorageFile(QQID, L"SavedCode.txt", GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING);
	if (hSavedFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			SendBackMessage(GroupID, QQID, L"诶？代码找不到了诶~");
		}
		else
		{
			SendBackMessage(GroupID, QQID, L"糟了！打开文件的时候出错了（⊙ｏ⊙）");
		}
		return 0;
	}
	

	//这里要格外小心。这个文件可能是用户写入过的
	CHAR* FileData = 0;
	WCHAR* WideCharStr = 0;

	BOOL bSuccess = 0;
	__try
	{
		DWORD FileSizeHigh;
		DWORD FileSizeLow = GetFileSize(hSavedFile, &FileSizeHigh);

		if (FileSizeHigh || FileSizeLow >= 65536)
		{
			SendBackMessage(GroupID, QQID, L"天哪这个代码文件怎么这么大？");
			SendBackMessage(GroupID, QQID, L"我不干了 我罢工了qaq");
			__leave;
		}
		else if (FileSizeLow == 0)
		{
			SendBackMessage(GroupID, QQID, L"这个代码文件怎么是空的哇");
			__leave;
		}

		FileData = malloc(FileSizeLow + 1);
		ZeroMemory(FileData, FileSizeLow + 1);

		DWORD BytesRead;
		ReadFile(hSavedFile, FileData, FileSizeLow, &BytesRead, 0);
		if (FileSizeLow != BytesRead)
		{
			SendBackMessage(GroupID, QQID, L"读取文件的时候出错了orz");
			__leave;
		}

		
		int cchWcLen = MultiByteToWideChar(54936, 0, FileData, FileSizeLow, 0, 0);
		WideCharStr = malloc(sizeof(WCHAR) * (cchWcLen + 1));
		ZeroMemory(WideCharStr, sizeof(WCHAR) * (cchWcLen + 1));

		MultiByteToWideChar(54936, 0, FileData, FileSizeLow, WideCharStr, cchWcLen);
		WideCharStr[cchWcLen] = 0;

		bSuccess = TRUE;
	}
	__finally
	{
		if (bSuccess == FALSE)
		{
			if (WideCharStr)free(WideCharStr);
		}
		if (FileData)free(FileData);
		CloseHandle(hSavedFile);
	}

	if (bSuccess)
	{
		RunCode(GroupID, QQID, SubType, AnonymousName, WideCharStr);
		free(WideCharStr);
	}
	return 0;
}