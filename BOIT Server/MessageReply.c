#include<Windows.h>
#include"MessageWatch.h"
#include"APITransfer.h"
#include"DirManagement.h"
#include<strsafe.h>
#include"EncodeConvert.h"
#include<string.h>
#include<wchar.h>

WCHAR * MatchReplyConfig(WCHAR ConfigFileName[], long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	HANDLE hCfgFile = CreateFileW(ConfigFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCfgFile == INVALID_HANDLE_VALUE) return FALSE;
	PBYTE pData = 0;
	WCHAR* pwData = 0;
	BOOL bMatch = FALSE;

	WCHAR*  Reply = 0;
	__try
	{
		DWORD FileSize = GetFileSize(hCfgFile, 0);
		pData = malloc(FileSize + 1);
		if (!pData)__leave;
		ZeroMemory(pData, FileSize + 1);
		DWORD BytesRead;
		if (ReadFile(hCfgFile, pData, FileSize, &BytesRead, NULL) == FALSE)__leave;
		if (BytesRead != FileSize)
		{
			__leave;
		}

		int wLen;
		pwData = StrConvMB2WC(CP_UTF8, pData, FileSize, &wLen);
		if (!pwData)
		{
			__leave;
		}

		//解析文件
		WCHAR* pwParse = pwData;

		BOOL ContentFound = 0, bMatch = FALSE,
			AllowUser = TRUE; //没写就是都允许

		while (wLen > 0)
		{
			int LineFeedLen = GetLineFeedLen(pwParse);
			wLen -= LineFeedLen;
			pwParse += LineFeedLen;

			int LineLen = GetLineLen(pwParse);
			//解析行
			if (LineLen)
			{
				if (pwParse[0] != L'#')//注释
				{
					WCHAR * FieldNameList[] = { L"Content",L"AllowUser",L"Match" };

					WCHAR* LineParse = pwParse;
					for (int i = 0; i < _countof(FieldNameList); i++)
					{
						if (_wcsnicmp(FieldNameList[i], LineParse, wcslen(FieldNameList[i])) == 0)
						{
							LineParse += wcslen(FieldNameList[i]);
							LineParse += GetLineSpaceLen(LineParse);
							if (LineParse[0] != L'=')
							{
								break;
							}
							LineParse++;
							LineParse += GetLineSpaceLen(LineParse);
							switch (i)
							{
							case 0:
								//解析Content
							{
								int LineLen = GetLineLen(LineParse);
								Reply = malloc(sizeof(WCHAR) * (LineLen + 1));
								wcsncpy_s(Reply, LineLen + 1, LineParse, LineLen);
								Reply[LineLen] = 0;

								
							}
							break;
							case 1:
								//解析AllowUser
							{
								AllowUser = FALSE;
								while (1)
								{
									int QQIDStrLen = GetCmdParamLen(LineParse);
									if ((!QQIDStrLen) || QQIDStrLen >= 16)break;

									WCHAR QQIDBuf[16] = { 0 };
									wcsncpy_s(QQIDBuf, _countof(QQIDBuf), LineParse, QQIDStrLen);
									long long RetvID;
									swscanf_s(QQIDBuf, L"%lld", &RetvID);
									if (RetvID == QQID)
									{
										AllowUser = TRUE; //匹配成功
										break;
									}
									LineParse += QQIDStrLen;
									LineParse += GetLineSpaceLen(LineParse);
								}
							}
							break;
							case 2:
							{
								int LineLen = GetLineLen(LineParse);
								
								if ((wcslen(Msg) == LineLen) && wcsncmp(Msg, LineParse, LineLen) == 0)
								{
									bMatch = TRUE;
								}
							}
							break;
							}

						}
					}


				}
			}

			pwParse += LineLen;
			wLen -= LineLen;
		}


		if (Reply && (AllowUser == FALSE || bMatch ==FALSE))
		{
			free(Reply);
			Reply = 0;
		}

	}
	__finally
	{
		if (hCfgFile)
		{
			CloseHandle(hCfgFile);
		}
		if (pData)
		{
			free(pData);
		}
		if (pwData)
		{
			free(pwData);
		}
	}
	return Reply;
}


WCHAR* TryFindReply(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	WIN32_FIND_DATAW FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bNext = TRUE;

	WCHAR MsgReplyPath[MAX_PATH];
	wcscpy_s(MsgReplyPath, MAX_PATH, GetBOITMsgReplyDir());
	PathAppendW(MsgReplyPath, L"*.cfg");

	WCHAR * Reply = 0;


	for (hFind = FindFirstFileW(MsgReplyPath, &FindData);
		hFind != INVALID_HANDLE_VALUE && wcslen(FindData.cFileName) > 0;
		bNext = FindNextFileW(hFind, &FindData))
	{
		if (!bNext)break;

		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{

			WCHAR CfgFilePath[MAX_PATH];
			wcscpy_s(CfgFilePath, MAX_PATH, GetBOITMsgReplyDir());
			PathAppendW(CfgFilePath, FindData.cFileName);

			Reply = MatchReplyConfig(CfgFilePath, GroupID, QQID, SubType, AnonymousName, Msg);
			
			if (Reply)break;
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);
	return Reply;
}




int MessageReplyWatch(long long MsgWatchID, PBYTE pData, UINT Event,
	long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR * wcReply = 0;
	if (wcslen(Msg) <= 16)
	{
		//试着匹配
		wcReply = TryFindReply(GroupID, QQID, SubType, AnonymousName, Msg);
	}

	if (wcReply)
	{
		if (GroupID)
		{
			SendGroupMessage(GroupID, wcReply);
		}
		else if (QQID)
		{
			SendPrivateMessage(QQID, wcReply);
		}
		free(wcReply);
		return BOIT_MSGWATCH_BLOCK;
	}

	return BOIT_MSGWATCH_PASS;
}



BOOL InitMessageReply()
{
	RegisterMessageWatch(BOIT_MW_ALL, -1, 0, MessageReplyWatch, 0);
	return 0;
}