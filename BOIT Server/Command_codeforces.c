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
#include<process.h>


#define QUERY_CF_CONTEST_TIME_INTERVAL (10  * 1000) // 10分钟

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


typedef struct __tagCFContestInfo
{
	int ContestID;
	WCHAR* ContestName;
	WCHAR* TypeString;
	WCHAR* PhaseString;
	long long DurationSeconds;
	long long StartTimeSeconds;
}CFCONTESTINFO, * pCFCONTESTINFO;


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

unsigned __stdcall CFQueryContestThread();
HANDLE hCFContestThread;
HANDLE hEventCFThreadCleanUp;


SRWLOCK CFContestsLock;
pVBUF CFContests = 0;
int ContestCount = 0;


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
		return 0;
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"输入#cf help查看帮助");
		return 0;
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
		
		QueryComingCurrentContests(boitSession);

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
		InitializeSRWLock(&CFContestsLock);
		CodeforcesInetInfo = AsyncINetInit(CFServerName);
		hEventCFThreadCleanUp = CreateEvent(0, TRUE, 0, 0);
		//启动线程
		hCFContestThread = _beginthreadex(0, 0, CFQueryContestThread, 0, 0, 0);
		break;

	case EC_CMDFREE:
		//等待线程结束

		SetEvent(hEventCFThreadCleanUp);
		WaitForSingleObject(hCFContestThread, INFINITE);
		CloseHandle(hEventCFThreadCleanUp);
		CloseHandle(hCFContestThread);
		AsyncINetCleanup(CodeforcesInetInfo);
		
		AcquireSRWLockExclusive(&CFContestsLock);
		if (CFContests)
		{
			for (int i = 0; i < ContestCount; i++)
			{
				FreeCFContest(((pCFCONTESTINFO)(CFContests->Data)) + i);
			}
			FreeVBuf(CFContests);
		}
		ReleaseSRWLockExclusive(&CFContestsLock);

		break;
	}
	return 0;
}

long long GetPresentUnixTime()
{
	SYSTEMTIME st;
	FILETIME ft;
	LARGE_INTEGER li;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	li.QuadPart /= 10000000;
	li.QuadPart -= 11644473600LL;
	return li.QuadPart;
}


BOOL QueryComingCurrentContests(pBOIT_SESSION boitSession)
{
	AcquireSRWLockShared(&CFContestsLock);
	
	if (CFContests)
	{
		//寻找正在进行的比赛
		int iOnGoing = 0;
		long long CurrentUnixTime = GetPresentUnixTime();
		pVBUF ReplyMsg = AllocVBuf();
		for (int i = 0; i < ContestCount; i++)
		{
			if (((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds < CurrentUnixTime &&
				((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds + ((pCFCONTESTINFO)(CFContests->Data))[i].DurationSeconds >= CurrentUnixTime)
			{
				iOnGoing++;
			}
		}

		if (iOnGoing == 0)
		{
			VBufferAppendStringW(ReplyMsg, L"现在没有正在进行的比赛");
		}
		else
		{
			WCHAR Buffer[32];
			swprintf_s(Buffer, _countof(Buffer), L"当前正在进行的比赛有 %d 个：", iOnGoing);
			VBufferAppendStringW(ReplyMsg, Buffer);

			for (int i = 0; i < ContestCount; i++)
			{
				if (((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds < CurrentUnixTime &&
					((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds + ((pCFCONTESTINFO)(CFContests->Data))[i].DurationSeconds >= CurrentUnixTime)
				{
					VBufferAppendStringW(ReplyMsg, L"\n");
					VBufferAppendStringW(ReplyMsg, ((pCFCONTESTINFO)(CFContests->Data))[i].ContestName);

					int TimeLeft = ((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds + ((pCFCONTESTINFO)(CFContests->Data))[i].DurationSeconds - CurrentUnixTime;
					VBufferAppendStringW(ReplyMsg, L"（剩余时间");
					if (TimeLeft / (24 * 60 * 60))
					{
						swprintf_s(Buffer, _countof(Buffer), L" %d 天", TimeLeft / (24 * 60 * 60));
						VBufferAppendStringW(ReplyMsg, Buffer);
						TimeLeft %= (24 * 60 * 60);
					}
					if (TimeLeft / (60 * 60))
					{
						swprintf_s(Buffer, _countof(Buffer), L" %d 小时", TimeLeft / (60 * 60));
						VBufferAppendStringW(ReplyMsg, Buffer);
						TimeLeft %= (60 * 60);
					}
					if (TimeLeft / (60))
					{
						swprintf_s(Buffer, _countof(Buffer), L" %d 分钟", TimeLeft / (60));
						VBufferAppendStringW(ReplyMsg, Buffer);
						TimeLeft %= (60);
					}
					if (TimeLeft)
					{
						swprintf_s(Buffer, _countof(Buffer), L" %d 秒", TimeLeft);
						VBufferAppendStringW(ReplyMsg, Buffer);
					}
					VBufferAppendStringW(ReplyMsg, L"）");
				}
			}
			
			
		}
		/*AddSizeVBuf(ReplyMsg, sizeof(WCHAR) * 1);
		((WCHAR*)(ReplyMsg->Data))[(ReplyMsg->Length / 2) - 1] = 0;
		SendBackMessage(boitSession, ReplyMsg->Data);
		FreeVBuf(ReplyMsg);*/

		//即将开始的比赛
		int TimeStamp = 0x7fffffff;
		/*ReplyMsg = AllocVBuf();*/
		for (int i = 0; i < ContestCount; i++)
		{
			if (((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds > CurrentUnixTime)
			{
				TimeStamp = min(TimeStamp, ((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds);
			}
		}

		if (TimeStamp == 0x7fffffff)
		{
			VBufferAppendStringW(ReplyMsg, L"\n\n最近没有什么比赛诶");
		}
		else
		{
			VBufferAppendStringW(ReplyMsg, L"\n\n最近将要开始的比赛有：");
			for (int i = 0; i < ContestCount; i++)
			{
				if (((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds == TimeStamp)
				{
					VBufferAppendStringW(ReplyMsg, L"\n");
					VBufferAppendStringW(ReplyMsg, ((pCFCONTESTINFO)(CFContests->Data))[i].ContestName);
					int TimeLeft = ((pCFCONTESTINFO)(CFContests->Data))[i].StartTimeSeconds - CurrentUnixTime;
					WCHAR Buffer[128];

					VBufferAppendStringW(ReplyMsg, L"（将于");
					if (TimeLeft < 10 * 30)
					{
						//预报分钟和秒
						if (TimeLeft / (60))
						{
							swprintf_s(Buffer, _countof(Buffer), L" %d 分钟", TimeLeft / (60));
							VBufferAppendStringW(ReplyMsg, Buffer);
							TimeLeft %= (60);
						}
						if (TimeLeft)
						{
							swprintf_s(Buffer, _countof(Buffer), L" %d 秒", TimeLeft);
							VBufferAppendStringW(ReplyMsg, Buffer);
						}
						VBufferAppendStringW(ReplyMsg, L"后开始，请做好参赛准备！）");
					}
					else
					{
						if (TimeLeft / (24 * 60 * 60))
						{
							swprintf_s(Buffer, _countof(Buffer), L" %d 天", TimeLeft / (24 * 60 * 60));
							VBufferAppendStringW(ReplyMsg, Buffer);
							TimeLeft %= (24 * 60 * 60);
						}
						if (TimeLeft / (60 * 60))
						{
							swprintf_s(Buffer, _countof(Buffer), L" %d 小时", TimeLeft / (60 * 60));
							VBufferAppendStringW(ReplyMsg, Buffer);
							TimeLeft %= (60 * 60);
						}
						if (TimeLeft / (60))
						{
							swprintf_s(Buffer, _countof(Buffer), L" %d 分钟", TimeLeft / (60));
							VBufferAppendStringW(ReplyMsg, Buffer);
							TimeLeft %= (60);
						}
						VBufferAppendStringW(ReplyMsg, L"后开始）");
					}
					
				}
			}
		}

		AddSizeVBuf(ReplyMsg, sizeof(WCHAR) * 1);
		((WCHAR*)(ReplyMsg->Data))[(ReplyMsg->Length / 2) - 1] = 0;
		SendBackMessage(boitSession, ReplyMsg->Data);
		FreeVBuf(ReplyMsg);
		//SendBackMessage(boitSession, ((pCFCONTESTINFO)(CFContests->Data))[0].ContestName);
	}
	else
	{
		SendBackMessage(boitSession, L"查询CF比赛信息失败了o(><；)oo");
	}
	ReleaseSRWLockShared(&CFContestsLock);
}


BOOL QueryCFUserInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
{
	WCHAR* UrlBuffer;
	int UTF8Len;
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, &UTF8Len);
	char* EncodedSearchStr;
	EncodedSearchStr = malloc((UTF8Len + 1) * 3); //最坏情况下每个字符都转义
	ZeroMemory(EncodedSearchStr, (UTF8Len + 1) * 3);
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, (UTF8Len + 1) * 3);
	free(UTF8Search);

	int WCSLen;
	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, &WCSLen);
	free(EncodedSearchStr);

	UrlBuffer = malloc((wcslen(L"/api/user.info?handles=") + WCSLen + 1) * sizeof(WCHAR));
	swprintf_s(UrlBuffer, (wcslen(L"/api/user.info?handles=") + WCSLen + 1), L"/api/user.info?handles=%ls", WCEncodedSearch);
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
	return 0;
}

int AsyncCFUserPhotoCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
	)
{
	pQUERY_CFPHOTO_SESS QueryPhotoSess = (pQUERY_CFPHOTO_SESS)ExtData;

	pVBUF ReplyBuffer = AllocVBuf();

	if (QueryPhotoSess->CFUserInfo.Handle)
	{
		VBufferAppendStringW(ReplyBuffer, L"昵称：");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Handle);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	/*if (QueryPhotoSess->CFUserInfo.FirstName && QueryPhotoSess->CFUserInfo.LastName)
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.FirstName);
		VBufferAppendStringW(ReplyBuffer, L"  ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.LastName);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}*/
	if (QueryPhotoSess->CFUserInfo.Country)
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Country);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	if ((QueryPhotoSess->CFUserInfo.Rating != -1) && (QueryPhotoSess->CFUserInfo.MaxRating != -1))
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
		wcscpy_s(PhotoFilePath, MAX_PATH, GetCQImageDir());
		PathAppendW(PhotoFilePath, PhotoFileName);
		HANDLE hFile = CreateFileW(PhotoFilePath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		DWORD BytesWrite;
		BOOL bSuccess = WriteFile(hFile, ReceivedBuf->Data, ReceivedBuf->Length, &BytesWrite, 0);
		//MessageBoxW(0, PhotoFilePath, bSuccess ? L"1" : L"0", 0);
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

	AsyncINetCleanup(QueryPhotoSess->CodeforcesPhotoInetInfo);

	//清理CF Profile



	if (ReplyBuffer->Length)
	{
		AddSizeVBuf(ReplyBuffer, sizeof(WCHAR) * 1);
		((WCHAR*)(ReplyBuffer->Data))[(ReplyBuffer->Length / 2) - 1] = 0;
		SendBackMessage(QueryPhotoSess->boitSession, ReplyBuffer->Data);
	}

	FreeBOITSession((pBOIT_SESSION)QueryPhotoSess->boitSession);

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
						else
						{
							CFQueryPhotoSess->CFUserInfo.Rating = -1;
						}
						if (ProfileField[5])
						{
							CFQueryPhotoSess->CFUserInfo.MaxRating = ProfileField[5]->valueint;
						}
						else
						{
							CFQueryPhotoSess->CFUserInfo.MaxRating = -1;
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


BOOL ParseCFContest(cJSON* ContestJson, pCFCONTESTINFO CFContest)
{
	char* ContestFieldName[] = { "id","name","type","phase" ,"durationSeconds","startTimeSeconds" };
	cJSON* ContestField[_countof(ContestFieldName)] = { 0 };

	for (cJSON* EnumContestField = ContestJson->child; EnumContestField; EnumContestField = EnumContestField->next)
	{
		for (int i = 0; i < _countof(ContestFieldName); i++)
		{
			if ((!ContestField[i]) && _strcmpi(EnumContestField->string, ContestFieldName[i]) == 0)
			{
				ContestField[i] = EnumContestField;
			}
		}
	}

	CFContest->ContestID = ContestField[0]->valueint;
	CFContest->ContestName = StrConvMB2WC(CP_UTF8, ContestField[1]->valuestring, -1, 0);
	CFContest->TypeString = StrConvMB2WC(CP_UTF8, ContestField[2]->valuestring, -1, 0);
	CFContest->PhaseString = StrConvMB2WC(CP_UTF8, ContestField[3]->valuestring, -1, 0);
	CFContest->DurationSeconds = ContestField[4]->valueint;
	if (ContestField[5])
	{
		CFContest->StartTimeSeconds = ContestField[5]->valueint;
	}
	else
	{
		CFContest->StartTimeSeconds = 0;
	}
	return 0;
}


BOOL FreeCFContest(pCFCONTESTINFO CFContest)
{
	free(CFContest->ContestName);
	free(CFContest->TypeString);
	free(CFContest->PhaseString);
}


BOOL ParseCFContestsInfoJson(char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);
	BOOL bSuccess = FALSE;
	int iContest = 0;
	pVBUF pContestArr = AllocVBuf();
	if (!JsonRoot)
	{
		//TODO: 这里log错误
		return 0;
	}
	__try
	{
		char* FieldName[] = { "status","result" };
		cJSON* JsonInfoField[_countof(FieldName)] = { 0 };
		for (cJSON* EnumField = JsonRoot->child; EnumField; EnumField = EnumField->next)
		{
			for (int i = 0; i < _countof(FieldName); i++)
			{
				if ((!JsonInfoField[i]) && _strcmpi(EnumField->string, FieldName[i]) == 0)
					JsonInfoField[i] = EnumField;
			}
		}

		if (JsonInfoField[0] && JsonInfoField[1])
		{
			if (_strcmpi(JsonInfoField[0]->valuestring, "OK") == 0)
			{
				for (cJSON* EnumContest = JsonInfoField[1]->child; EnumContest; EnumContest = EnumContest->next)
				{
					AddSizeVBuf(pContestArr, sizeof(CFCONTESTINFO));

					ParseCFContest(EnumContest, ((pCFCONTESTINFO)(pContestArr->Data)) + iContest);

					iContest++;
				}

				bSuccess = TRUE;
			}
		}
	}
	__finally
	{
		cJSON_Delete(JsonRoot);

		if (!bSuccess)
		{
			FreeVBuf(pContestArr);
		}
		else
		{
			//开个全局变量，上个锁，替换掉
			pVBUF OldContests;
			int OldContestCount;
			AcquireSRWLockExclusive(&CFContestsLock);

			OldContests = CFContests;
			OldContestCount = ContestCount;
			CFContests = pContestArr;
			ContestCount = iContest;

			ReleaseSRWLockExclusive(&CFContestsLock);
			if (OldContests)
			{
				for (int i = 0; i < OldContestCount; i++)
				{
					FreeCFContest(((pCFCONTESTINFO)(OldContests->Data)) + i);
				}
				FreeVBuf(OldContests);
			}
			
		}
	}
	return 0;
}


int AsyncCFContestCallback(
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
		ParseCFContestsInfoJson(ReceivedBuf->Data);
		break;
	case ASYNCINET_REASON_FAILED:
		Sleep(0);
		break;
	}
	return 0;
}




VOID __stdcall CFContestQueryTimerProc(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
	)
{
	AsyncRequestGet(CodeforcesInetInfo, L"/api/contest.list?gym=false", 0, AsyncCFContestCallback);
	return 0;
}

unsigned __stdcall CFQueryContestThread()
{
	HANDLE hTimer = CreateWaitableTimer(0, 0, 0);
	LARGE_INTEGER LargeInteger;
	LargeInteger.QuadPart = -1;
	SetWaitableTimer(hTimer, &LargeInteger, QUERY_CF_CONTEST_TIME_INTERVAL, CFContestQueryTimerProc, 0, 0);
	while (1)
	{
		DWORD x = WaitForSingleObjectEx(hEventCFThreadCleanUp, INFINITE, TRUE);
		if (x == WAIT_OBJECT_0)
		{
			break;
		}
	}

	CloseHandle(hTimer);
	return 0;
}