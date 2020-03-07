#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include<WinInet.h>
#include"VBuffer.h"
#include"cJSON.h"
#include<strsafe.h>
#include"EncodeConvert.h"
#include<shlwapi.h>
#include"DirManagement.h"
#include"URIEncode.h"
#include"Corpus.h"
#pragma comment(lib,"WinINet.lib")


#define CFUSER_QRY_BUFSZ 4096

#define OIER_MAX_DISPLAY 3
typedef struct __tagQueryCFUser
{
	pBOIT_SESSION boitSession;
	BYTE ReadBuffer[CFUSER_QRY_BUFSZ];
	DWORD BytesRead;

	HINTERNET hRequest;
	pVBUF vBuffer;
	BOOL bRequestComplete;
}QUERY_CFUSER_STRUCT, * pQUERY_CFUSER_STRUCT;

WCHAR CFServerName[] = L"codeforces.com";

HINTERNET hInet;
HINTERNET hConnect;

VOID CALLBACK QueryCFUserCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
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
		WCHAR * OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
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
		hInet = InternetOpenW(L"BOIT", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
		INTERNET_STATUS_CALLBACK pOldStatusCallback = InternetSetStatusCallbackW(hInet, QueryCFUserCallback);

		// For HTTP InternetConnect returns synchronously because it does not
		// actually make the connection.

		hConnect = InternetConnectW(hInet,
			CFServerName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		break;


	case EC_CMDFREE:
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInet);
		break;
	}
	return 0;
}


pQUERY_CFUSER_STRUCT AllocQueryCFUserStruct(pBOIT_SESSION boitSession)
{
	pQUERY_CFUSER_STRUCT QueryStruct = malloc(sizeof(QUERY_CFUSER_STRUCT));
	ZeroMemory(QueryStruct, sizeof(QUERY_CFUSER_STRUCT));

	QueryStruct->boitSession = DuplicateBOITSession(boitSession);

	QueryStruct->vBuffer = AllocVBuf();

	return QueryStruct;
}


BOOL FreeQueryCFUserStruct(pQUERY_CFUSER_STRUCT QueryStruct)
{
	FreeBOITSession(QueryStruct->boitSession);
	FreeVBuf(QueryStruct->vBuffer);
	free(QueryStruct);
	return TRUE;
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
	pQUERY_CFUSER_STRUCT QueryStruct = AllocQueryCFUserStruct(boitSession);
	WCHAR* rgpszAcceptTypes[] = { L"*/*", NULL };
	QueryStruct->hRequest = HttpOpenRequestW(hConnect, L"GET", UrlBuffer,
		NULL, NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, QueryStruct);

	BOOL x = HttpSendRequestW(QueryStruct->hRequest, 0, 0, 0, 0);
	return TRUE;
}



VOID CALLBACK QueryCFUserCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
)
{
	pQUERY_CFUSER_STRUCT QueryStruct = dwContext;
	BOOL bSuccess = FALSE;
	HINTERNET* a = lpvStatusInformation;
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_REQUEST_COMPLETE:
	{
		INTERNET_ASYNC_RESULT* AsyncResult = lpvStatusInformation;
		if (AsyncResult->dwResult == 0)
		{
			//Failed

		}
		else
		{
			if (!QueryStruct->bRequestComplete)
			{
				QueryStruct->bRequestComplete = TRUE;
			}
			else
			{
				//上次结果复制进缓冲区
				//if (QueryStruct->InetBuf.dwBufferLength < 1000)
			ReadNow:
				{
					int OrgLen = QueryStruct->vBuffer->Length;
					AddSizeVBuf(QueryStruct->vBuffer, QueryStruct->BytesRead);
					memcpy(QueryStruct->vBuffer->Data + OrgLen, QueryStruct->ReadBuffer, QueryStruct->BytesRead);
				}
			}


			//QueryStruct->InetBuf.dwBufferLength = CFUSER_QRY_BUFSZ;
			ZeroMemory(QueryStruct->ReadBuffer, CFUSER_QRY_BUFSZ);
			//BOOL bRet = InternetReadFile(QueryStruct->hRequest, &(QueryStruct->InetBuf), IRF_ASYNC, QueryStruct);

			BOOL bRet = InternetReadFile(QueryStruct->hRequest, QueryStruct->ReadBuffer, CFUSER_QRY_BUFSZ, &(QueryStruct->BytesRead));
			DWORD dwErr = GetLastError();
			if (!bRet)
			{
				if (dwErr == ERROR_IO_PENDING)
				{
					//如果是ERROR_IO_PENDING的话，继续下去等待执行完毕即可
					return;
				}
			}
			else
			{
				if (QueryStruct->BytesRead)
				{
					goto ReadNow;
				}
				//操作结束了，末尾补一个0
				AddSizeVBuf(QueryStruct->vBuffer, 1);
				QueryStruct->vBuffer->Data[QueryStruct->vBuffer->Length - 1] = 0;
				bSuccess = TRUE;

				//解析json
				ParseCFUserInfoJsonAndSend(QueryStruct->boitSession, QueryStruct->vBuffer->Data);
			}
		}

		//控制流流到这里的不是失败了就是结束了。清理。
		if (!bSuccess)
		{
			//失败通知
			SendBackMessage(QueryStruct->boitSession, L"哎呀，查询CF用户失败了");
		}
		InternetCloseHandle(QueryStruct->hRequest);
		FreeQueryCFUserStruct(QueryStruct);

	}
	}
	return;
}



BOOL ParseCFUserInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);

	if (!JsonRoot)
	{
		SendBackMessage(boitSession, L"看上去出了点小问题...");
		return 0;
	}
	pVBUF ResultStr = AllocVBuf();

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
						"maxRank" // 7
					};
					cJSON* ProfileField[_countof(ProfileFieldName)] = { 0 };
					for (cJSON* ProfileFieldEnum = JsonInfoField[2]->child->child; ProfileFieldEnum; ProfileFieldEnum = ProfileFieldEnum->next)
					{
						for (int i = 0; i < _countof(ProfileFieldName); i++)
						{
							if ((!ProfileField[i]) && _strcmpi(ProfileFieldEnum->string, ProfileFieldName[i]) == 0)
								ProfileField[i] = ProfileFieldEnum;
						}
					}

					if (ProfileField[0])
					{
						WCHAR* NickString = StrConvMB2WC(CP_UTF8, ProfileField[0]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, L"昵称：");
						VBufferAppendStringW(ResultStr, NickString);
						VBufferAppendStringW(ResultStr, L"\n");
						free(NickString);
					}
					if (ProfileField[1] && ProfileField[2])
					{
						WCHAR* firstNameString = StrConvMB2WC(CP_UTF8, ProfileField[2]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, firstNameString);
						VBufferAppendStringW(ResultStr, L"  ");
						free(firstNameString);

						WCHAR* lastNameString = StrConvMB2WC(CP_UTF8, ProfileField[1]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, lastNameString);
						VBufferAppendStringW(ResultStr, L"\n");
						free(lastNameString);
					}
					if (ProfileField[3])
					{
						WCHAR* CountryStr = StrConvMB2WC(CP_UTF8, ProfileField[3]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, CountryStr);
						VBufferAppendStringW(ResultStr, L"\n");
						free(CountryStr);
					}
					if (ProfileField[4] && ProfileField[5])
					{
						WCHAR Str[32];
						swprintf_s(Str, _countof(Str), L"rating: %d(max: %d)\n", ProfileField[4]->valueint, ProfileField[5]->valueint);
						VBufferAppendStringW(ResultStr, Str);
					}
					if (ProfileField[6] && ProfileField[7])
					{
						WCHAR* RankStr = StrConvMB2WC(CP_UTF8, ProfileField[6]->valuestring, -1, 0);
						WCHAR* MaxRankStr = StrConvMB2WC(CP_UTF8, ProfileField[7]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, L"rank: ");
						VBufferAppendStringW(ResultStr, RankStr);
						VBufferAppendStringW(ResultStr, L"(max: ");
						VBufferAppendStringW(ResultStr, MaxRankStr);
						VBufferAppendStringW(ResultStr, L")\n");
						free(RankStr);
						free(MaxRankStr);
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
		if (ResultStr->Length)
		{
			AddSizeVBuf(ResultStr, sizeof(WCHAR) * 1);
			((WCHAR*)(ResultStr->Data))[(ResultStr->Length / 2) - 1] = 0;
			SendBackMessage(boitSession, ResultStr->Data);
		}

		cJSON_Delete(JsonRoot);
		FreeVBuf(ResultStr);
	}
	return 0;
}

