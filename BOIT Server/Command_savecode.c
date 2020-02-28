#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"DirManagement.h"
#include"EncodeConvert.h"

int CmdMsg_savecode_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);
	
	if (ParamLen + SpaceLen == wcslen(Msg))
	{
		SendBackMessage(GroupID, QQID, L"诶？你的代码呢？qwq");
		return 0;
	}

	Msg += ParamLen + SpaceLen;
	//写入文件
	HANDLE hSavedFile = PerUserCreateStorageFile(QQID, L"SavedCode.txt", GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS);
	if (hSavedFile == INVALID_HANDLE_VALUE)
	{
		SendBackMessage(GroupID, QQID, L"写入文件的时候出错了qaq");
		return 0;
	}

	char* UTF8Text = 0;
	BOOL bSuccessSave = FALSE;
	__try
	{
		//为了打开读取方便，这里转码成UTF-8
		int cbUTF8Len;
		UTF8Text = StrConvWC2MB(CP_UTF8, Msg, -1, &cbUTF8Len);

		if (!UTF8Text)
		{
			__leave;
		}
		
		DWORD BytesWritten;
		WriteFile(hSavedFile, UTF8Text, cbUTF8Len, &BytesWritten, 0);
		if (BytesWritten != cbUTF8Len)
		{
			__leave;
		}
		bSuccessSave = TRUE;
	}
	__finally
	{
		if (UTF8Text) free(UTF8Text);
		CloseHandle(hSavedFile);
	}

	if (bSuccessSave == TRUE)
	{
		SendBackMessage(GroupID, QQID, L"保存代码成功！");
		SendBackMessage(GroupID, QQID, L"非常抱歉bot还在开发周期中，您的代码不一定会被始终保存！");
	}
	else
	{
		SendBackMessage(GroupID, QQID, L"糟了！写入文件的时候失败了！");
	}
	return 0;
}