#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"VBuffer.h"
#include"cJSON.h"
#include<strsafe.h>
#include"EncodeConvert.h"
#include<shlwapi.h>
#include"DirManagement.h"
#include"URIEncode.h"
#include"Corpus.h"
#include"AsyncINet.h"



typedef struct __tagCFUserInfo
{
	WCHAR* Handle;
	WCHAR* LastName;
	WCHAR* FirstName;
	WCHAR* Country;
	int Rating; // 无数据为 -1
	int MaxRating; // 无数据为 -1
	WCHAR* Rank;
	WCHAR* MaxRank;
	WCHAR* TitlePhotoURL;
}CFUSERINFO, * pCFUSERINFO;


typedef struct __tagQueryCFPhotoSession
{
	CFUSERINFO CFUserInfo;
	pBOIT_SESSION boitSession;
	pASYNCINET_INFO CodeforcesPhotoInetInfo;
}QUERY_CFPHOTO_SESS, * pQUERY_CFPHOTO_SESS;

pASYNCINET_INFO CodeforcesInetInfo;




WCHAR CFServerName[] = L"codeforces.com";

static HINTERNET hInet;
static HINTERNET hConnect;

int AsyncCFUserCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
);


int CmdMsg_codeforces_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	//if (boitSession->GroupID && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryCFUser") == 0)
	//{
	//	SendBackMessage(boitSession, L"该群禁止了查询CF User功能。请在私聊中查询。");
	//	return 0;
	//}



	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int OrderStrlen = GetCmdParamLen(Msg + ParamLen + SpaceLen);

	if (OrderStrlen >= 64)
	{
		SendBackMessage(boitSession, L"无法识别指令");
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"输入#cf help查看帮助");
	}

	WCHAR* OrderStr[] = { L"help",L"profile",L"contests" };
	int iMatch;
	for (iMatch = 0; iMatch < _countof(OrderStr); iMatch++)
	{
		if (OrderStrlen == wcslen(OrderStr[iMatch]) && (_wcsnicmp(Msg + ParamLen + SpaceLen, OrderStr[iMatch], OrderStrlen) == 0))
		{
			break;
		}
	}
	switch (iMatch)
	{
	case 0:
		SendBackMessage(boitSession, L"查询有关codeforces的信息。\n#cf profile [用户名] 以查询profile\n#cf contests 以查询比赛信息\n#cf help 以查看该帮助信息");

		break;
	case 1:
	{
		WCHAR* OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
		OrderBase += GetCmdSpaceLen(OrderBase);
		int QueryStrlen = GetCmdParamLen(OrderBase);
		if (QueryStrlen >= 64)
		{
			SendBackMessage(boitSession, L"这是哪个CF用户啊，名字这么长？");
		}
		else if (QueryStrlen == 0)
		{
			SendBackMessage(boitSession, L"你要查询的是哪个CF用户诶？");
		}
		else
		{
			SendBackMessage(boitSession, L"查询中...这可能要一点时间");
			QueryCFUserInfo(boitSession, OrderBase);
		}
	}

	break;
	case 2:
		SendBackMessage(boitSession, Corpus_FunctionDeving());

		break;
	default:
		SendBackMessage(boitSession, L"未找到指令。输入#cf help查看帮助");
		break;
	}
	/*else
	{

	}*/

	return 0;
}

int CmdEvent_codeforces_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_CMDLOAD:
		CodeforcesInetInfo = AsyncINetInit(CFServerName);
		break;

	case EC_CMDFREE:
		AsyncINetCleanup(CodeforcesInetInfo);
		break;
	}
	return 0;
}

BOOL QueryCFUserInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
{
	WCHAR UrlBuffer[256];
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, 0);
	char EncodedSearchStr[256];
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, _countof(EncodedSearchStr));
	free(UTF8Search);

	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, 0);
	swprintf_s(UrlBuffer, _countof(UrlBuffer), L"/api/user.info?handles=%s", WCEncodedSearch);
	free(WCEncodedSearch);


	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(CodeforcesInetInfo, UrlBuffer, newBoitSess, AsyncCFUserCallback);
	return TRUE;
}


int AsyncCFUserCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
)
{
	switch (iReason)
	{
	case ASYNCINET_REASON_SUCCESS:
		AddSizeVBuf(ReceivedBuf, 1);
		ReceivedBuf->Data[ReceivedBuf->Length - 1] = 0;

		//解析json
		ParseCFUserInfoJsonAndSend((pBOIT_SESSION)ExtData, ReceivedBuf->Data);
		//这里可能会继续查询头像，所以不删除boitSession
		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"哎呀，查询CF用户失败了");
		FreeBOITSession((pBOIT_SESSION)ExtData);
		break;
	}
	
}

int AsyncCFUserPhotoCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
)
{
	pQUERY_CFPHOTO_SESS QueryPhotoSess = (pQUERY_CFPHOTO_SESS)ExtData;

	pVBUF ReplyBuffer = AllocVBuf();
	
	if (QueryPhotoSess->CFUserInfo.Handle[0])
	{
		VBufferAppendStringW(ReplyBuffer, L"昵称：");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Handle);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	/*if (QueryPhotoSess->CFUserInfo.FirstName[0] && QueryPhotoSess->CFUserInfo.LastName)
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.FirstName);
		VBufferAppendStringW(ReplyBuffer, L"  ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.LastName);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}*/
	if (QueryPhotoSess->CFUserInfo.Country[0])
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Country);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	if ((QueryPhotoSess->CFUserInfo.Rating != -1)&& (QueryPhotoSess->CFUserInfo.MaxRating!=-1))
	{
		WCHAR Str[32];
		swprintf_s(Str, _countof(Str), L"rating: %d(max: %d)\n", QueryPhotoSess->CFUserInfo.Rating, QueryPhotoSess->CFUserInfo.MaxRating);
		VBufferAppendStringW(ReplyBuffer, Str);
	}
	if (QueryPhotoSess->CFUserInfo.Rank && QueryPhotoSess->CFUserInfo.MaxRank)
	{
		VBufferAppendStringW(ReplyBuffer, L"rank: ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Rank);
		VBufferAppendStringW(ReplyBuffer, L"(max: ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.MaxRank);
		VBufferAppendStringW(ReplyBuffer, L")\n");
	}

	switch (iReason)
	{
	case ASYNCINET_REASON_SUCCESS:
	{
		WCHAR PhotoFileName[MAX_PATH];
		CoolQAllocPicFileName(PhotoFileName);

		WCHAR PhotoFilePath[MAX_PATH];
		wcscpy_s(PhotoFilePath,MAX_PATH,GetCQImageDir());
		PathAppendW(PhotoFilePath, PhotoFileName);
		HFILE hFile = CreateFileW(PhotoFilePath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		DWORD BytesWrite;
		BOOL bSuccess = WriteFile(hFile, ReceivedBuf->Data, ReceivedBuf->Length, &BytesWrite, 0);
		CloseHandle(hFile);
		
		WCHAR TestBuffer[32];

		swprintf_s(TestBuffer, _countof(TestBuffer), L"[CQ:image,file=%ls]", PhotoFileName);
		VBufferAppendStringW(ReplyBuffer, TestBuffer);
	}
		
		break;
	case ASYNCINET_REASON_FAILED:
		VBufferAppendStringW(ReplyBuffer, L"抓取头像失败了orz");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)QueryPhotoSess->boitSession);
	AsyncINetCleanup(QueryPhotoSess->CodeforcesPhotoInetInfo);

	//清理CF Profile
	
	

	if (ReplyBuffer->Length)
	{
		AddSizeVBuf(ReplyBuffer, sizeof(WCHAR) * 1);
		((WCHAR*)(ReplyBuffer->Data))[(ReplyBuffer->Length / 2) - 1] = 0;
		SendBackMessage(QueryPhotoSess->boitSession, ReplyBuffer->Data);
	}

	{
		free(QueryPhotoSess->CFUserInfo.Country);
		free(QueryPhotoSess->CFUserInfo.FirstName);
		free(QueryPhotoSess->CFUserInfo.Handle);
		free(QueryPhotoSess->CFUserInfo.LastName);
		free(QueryPhotoSess->CFUserInfo.MaxRank);
		free(QueryPhotoSess->CFUserInfo.Rank);
		free(QueryPhotoSess->CFUserInfo.TitlePhotoURL);
	}
	free(QueryPhotoSess);

	FreeVBuf(ReplyBuffer);
	return 0;
}


BOOL ParseCFUserInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);

	if (!JsonRoot)
	{
		SendBackMessage(boitSession, L"看上去出了点小问题...");
		return 0;
	}

	__try
	{
		char* FieldName[] = { "status", "comment","result" };
		cJSON* JsonInfoField[_countof(FieldName)] = { 0 };
		for (cJSON* EnumField = JsonRoot->child; EnumField; EnumField = EnumField->next)
		{
			for (int i = 0; i < _countof(FieldName); i++)
			{
				if ((!JsonInfoField[i]) && _strcmpi(EnumField->string, FieldName[i]) == 0)
					JsonInfoField[i] = EnumField;
			}
		}

		if (JsonInfoField[0])
		{
			if (_strcmpi(JsonInfoField[0]->valuestring, "OK") == 0)
			{
				if (JsonInfoField[2] && JsonInfoField[2]->child && JsonInfoField[2]->child->child)
				{
					char* ProfileFieldName[] = { "handle" ,"lastName", "firstName",
						"country",// 3
						"rating",// 4
						"maxRating",// 5
						"rank",// 6
						"maxRank", // 7
						"titlePhoto" // 8
					};
					cJSON* ProfileField[_countof(ProfileFieldName)] = { 0 };

					int iMatchProfileField = 0;
					for (cJSON* ProfileFieldEnum = JsonInfoField[2]->child->child; ProfileFieldEnum; ProfileFieldEnum = ProfileFieldEnum->next)
					{
						for (int i = 0; i < _countof(ProfileFieldName); i++)
						{
							if ((!ProfileField[i]) && _strcmpi(ProfileFieldEnum->string, ProfileFieldName[i]) == 0)
							{
								ProfileField[i] = ProfileFieldEnum;
								iMatchProfileField++;
							}
						}
					}

					if (iMatchProfileField)
					{
						pQUERY_CFPHOTO_SESS CFQueryPhotoSess = malloc(sizeof(QUERY_CFPHOTO_SESS));
						ZeroMemory(CFQueryPhotoSess, sizeof(QUERY_CFPHOTO_SESS));

						CFQueryPhotoSess->boitSession = boitSession;

						if (ProfileField[0])
						{
							CFQueryPhotoSess->CFUserInfo.Handle = StrConvMB2WC(CP_UTF8, ProfileField[0]->valuestring, -1, 0);
						}
						if (ProfileField[1])
						{
							CFQueryPhotoSess->CFUserInfo.LastName = StrConvMB2WC(CP_UTF8, ProfileField[1]->valuestring, -1, 0);
						}
						if (ProfileField[2])
						{
							CFQueryPhotoSess->CFUserInfo.FirstName = StrConvMB2WC(CP_UTF8, ProfileField[2]->valuestring, -1, 0);
						}
						if (ProfileField[3])
						{
							CFQueryPhotoSess->CFUserInfo.Country = StrConvMB2WC(CP_UTF8, ProfileField[3]->valuestring, -1, 0);
						}
						if (ProfileField[4])
						{
							CFQueryPhotoSess->CFUserInfo.Rating = ProfileField[4]->valueint;
						}
						if (ProfileField[5])
						{
							CFQueryPhotoSess->CFUserInfo.MaxRating = ProfileField[5]->valueint;
						}
						if (ProfileField[6])
						{
							CFQueryPhotoSess->CFUserInfo.Rank = StrConvMB2WC(CP_UTF8, ProfileField[6]->valuestring, -1, 0);
						}
						if (ProfileField[7])
						{
							CFQueryPhotoSess->CFUserInfo.MaxRank = StrConvMB2WC(CP_UTF8, ProfileField[7]->valuestring, -1, 0);
						}
						if (ProfileField[8])
						{
							CFQueryPhotoSess->CFUserInfo.TitlePhotoURL = StrConvMB2WC(CP_UTF8, ProfileField[8]->valuestring, -1, 0);
						}
						WCHAR FormatTitlePhotoURL[256];
						WCHAR FormatTitlePhotoHostName[256];
						WCHAR URLQueryPart[256];
						DWORD dURLPartLen;

						//拼接URL
						WCHAR Scheme[] = L"http:";
						swprintf(FormatTitlePhotoURL, _countof(FormatTitlePhotoURL), L"%s%ls?a=1", Scheme, CFQueryPhotoSess->CFUserInfo.TitlePhotoURL);

						dURLPartLen = _countof(FormatTitlePhotoHostName);
						UrlGetPartW(FormatTitlePhotoURL, FormatTitlePhotoHostName, &dURLPartLen, URL_PART_HOSTNAME, 0);

						dURLPartLen = _countof(URLQueryPart);
						UrlGetPartW(FormatTitlePhotoURL, URLQueryPart, &dURLPartLen, URL_PART_QUERY, 0);

						CFQueryPhotoSess->CodeforcesPhotoInetInfo = AsyncINetInit(FormatTitlePhotoHostName);
						AsyncRequestGet(CFQueryPhotoSess->CodeforcesPhotoInetInfo,
							FormatTitlePhotoURL + wcslen(Scheme) + wcslen(L"//") + wcslen(FormatTitlePhotoHostName),
							CFQueryPhotoSess, AsyncCFUserPhotoCallback);
					}
					else
					{
						SendBackMessage(boitSession, L"看上去出了点小问题...");
					}
				}
				else
				{
					SendBackMessage(boitSession, L"看上去出了点小问题...");
				}
			}
			else if (_strcmpi(JsonInfoField[0]->valuestring, "FAILED") == 0)
			{
				SendBackMessage(boitSession, L"什么都没找到诶 再试试看？");
			}
			else
			{
				SendBackMessage(boitSession, L"看上去出了点小问题...");
				__leave;
			}
		}
	}
	__finally
	{
		cJSON_Delete(JsonRoot);
	}
	return 0;
}

