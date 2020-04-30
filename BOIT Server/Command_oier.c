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
#include"AsyncINet.h"



#define OIER_QRY_BUFSZ 4096

#define OIER_MAX_DISPLAY 3
typedef struct __tagQueryOIerStruct
{
	pBOIT_SESSION boitSession;
	BYTE ReadBuffer[OIER_QRY_BUFSZ];
	DWORD BytesRead;

	HINTERNET hRequest;
	pVBUF vBuffer;
	BOOL bRequestComplete;
}QUERY_OIER_STRUCT, * pQUERY_OIER_STRUCT;

WCHAR OIerDBServerName[] = L"bytew.net";

pASYNCINET_INFO OIerDBInetInfo;

int AsyncOIerInfoCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
);

int CmdMsg_oier_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if ((GetBOITSessionType(boitSession) == BOITSESS_TYPE_GROUP) && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryOIer") == 0)
	{
		SendBackMessage(boitSession, L"该群禁止了查询OIer功能。请在私聊中查询或联系管理员开放功能。");
		return 0;
	}



	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int QueryStrlen = wcslen(Msg + ParamLen + SpaceLen);
	if (QueryStrlen >= 64)
	{
		SendBackMessage(boitSession, L"这是哪个OIer啊，名字这么长？");
	}
	else if (QueryStrlen == 0)
	{
		SendBackMessage(boitSession, L"你要查询的是哪个OIer诶？（试试 #oier 人名/拼音）");
	}
	else
	{
		QueryOIerInfo(boitSession, Msg + ParamLen + SpaceLen);
	}

	return 0;
}

int CmdEvent_oier_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_CMDLOAD:
		OIerDBInetInfo = AsyncINetInit(OIerDBServerName);
		break;

	case EC_CMDFREE:
		AsyncINetCleanup(OIerDBInetInfo);
		break;
	}
	return 0;
}







BOOL QueryOIerInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
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

	UrlBuffer = malloc((wcslen(L"/OIer/search.php?method=normal&q=") + WCSLen + 1) * sizeof(WCHAR));
	swprintf_s(UrlBuffer, (wcslen(L"/OIer/search.php?method=normal&q=") + WCSLen + 1), L"/OIer/search.php?method=normal&q=%ls", WCEncodedSearch);
	free(WCEncodedSearch);


	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(OIerDBInetInfo, UrlBuffer, newBoitSess, AsyncOIerInfoCallback);


	/*WCHAR UrlBuffer[256 + 32] = { 0 };
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, 0);
	char EncodedSearchStr[256 + 1] = { 0 };
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, _countof(EncodedSearchStr));
	free(UTF8Search);

	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, 0);
	swprintf_s(UrlBuffer, _countof(UrlBuffer), L"/OIer/search.php?method=normal&q=%ls", WCEncodedSearch);
	free(WCEncodedSearch);
	
	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(OIerDBInetInfo, UrlBuffer, newBoitSess, AsyncOIerInfoCallback);*/
	return TRUE;
}


int AsyncOIerInfoCallback(
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
		ParseOIerInfoJsonAndSend((pBOIT_SESSION)ExtData, ReceivedBuf->Data);
		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"哎呀，查询OIer失败了");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)ExtData);
	return 0;
}



BOOL ParseOIerInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	//测试
	cJSON* JsonRoot = cJSON_Parse(JsonData);
	int TotalResult = 0;
	pVBUF PerPersonResult[OIER_MAX_DISPLAY] = { 0 };
	__try
	{
		if (JsonRoot && JsonRoot->child && JsonRoot->child->child)
		{
			for (cJSON* EnumPerson = JsonRoot->child->child; EnumPerson; EnumPerson = EnumPerson->next)
			{

				if (TotalResult >= OIER_MAX_DISPLAY)
				{
					TotalResult++;
					continue;
				}
				PerPersonResult[TotalResult] = AllocVBuf();

				char* InfoField[] = { "name", "sex","awards" };
				cJSON* JsonInfoField[_countof(InfoField)] = { 0 };
				for (cJSON* EnumField = EnumPerson->child; EnumField; EnumField = EnumField->next)
				{
					for (int i = 0; i < _countof(InfoField); i++)
					{
						if ((!JsonInfoField[i]) && _strcmpi(EnumField->string, InfoField[i]) == 0)
							JsonInfoField[i] = EnumField;
					}
				}

				if (JsonInfoField[0]) // 姓名
				{
					VBufferAppendStringW(PerPersonResult[TotalResult], L"姓名：");
					WCHAR* NameString = StrConvMB2WC(CP_UTF8, JsonInfoField[0]->valuestring, -1, 0);
					VBufferAppendStringW(PerPersonResult[TotalResult], NameString);
					free(NameString);

					VBufferAppendStringW(PerPersonResult[TotalResult], L"  ");

					if (JsonInfoField[1])
					{
						WCHAR* SexNameList[] = { L"女",L"未知",L"男" };
						int SexIndex = atoi(JsonInfoField[1]->valuestring);
						if (SexIndex == 1 || SexIndex == -1)
						{
							VBufferAppendStringW(PerPersonResult[TotalResult], L"生理性别：");
							VBufferAppendStringW(PerPersonResult[TotalResult], SexNameList[SexIndex + 1]);
							VBufferAppendStringW(PerPersonResult[TotalResult], L"  ");
						}
					}
					if (JsonInfoField[2])
					{
						char* AwardsStr = JsonInfoField[2]->valuestring;
						int AwardStrlen = strlen(AwardsStr);
						for (int i = 0; i < AwardStrlen; i++)
						{
							if (AwardsStr[i] == '\'')
							{
								AwardsStr[i] = '\"';
							}
						}
						cJSON* ParseAward = cJSON_Parse(AwardsStr);

						if (ParseAward && ParseAward->child)
						{
							for (cJSON* EnumAward = ParseAward->child; EnumAward; EnumAward = EnumAward->next)
							{
								char* AwardField[] = { "rank","province","award_type","identity","school","grade","score" };
								cJSON* JsonAwardField[_countof(AwardField)] = { 0 };

								for (cJSON* EnumAwardField = EnumAward->child; EnumAwardField; EnumAwardField = EnumAwardField->next)
								{
									for (int i = 0; i < _countof(AwardField); i++)
									{
										if ((!JsonAwardField[i]) && _strcmpi(EnumAwardField->string, AwardField[i]) == 0)
											JsonAwardField[i] = EnumAwardField;
									}
								}

								VBufferAppendStringW(PerPersonResult[TotalResult], L"\n");

								if (JsonAwardField[5])//年级
								{
									VBufferAppendStringW(PerPersonResult[TotalResult], L"于");
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[5]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"时");
									free(Str);
								}
								VBufferAppendStringW(PerPersonResult[TotalResult], L"在");
								if (JsonAwardField[1])//省份
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[1]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L" ");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L" ");
									free(Str);
								}
								if (JsonAwardField[4])//学校
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[4]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									free(Str);
								}
								if (JsonAwardField[3])//比赛名称
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[3]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"参加");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"，");
									free(Str);
								}
								if (JsonAwardField[6] && JsonAwardField[2])//分数 + 奖项
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[6]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"以");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"的成绩取得");
									free(Str);
									Str = StrConvMB2WC(CP_UTF8, JsonAwardField[2]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									free(Str);
								}
								else if (JsonAwardField[6] || JsonAwardField[2])//分数 or 奖项
								{
									cJSON* ChosenField = JsonAwardField[6] ? JsonAwardField[6] : JsonAwardField[2];
									WCHAR* Str = StrConvMB2WC(CP_UTF8, ChosenField->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"取得");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"的成绩");
									free(Str);
								}
								if (JsonAwardField[0])//排名
								{
									WCHAR Str[8];
									swprintf_s(Str, _countof(Str), L"%d", JsonAwardField[0]->valueint);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"，排名");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
								}
								VBufferAppendStringW(PerPersonResult[TotalResult], L"。");
							}
						}


						if (ParseAward)cJSON_Delete(ParseAward);
					}
					TotalResult++;
				}
			}
		}
	}
	__finally
	{
		if (TotalResult == 0)
		{
			SendBackMessage(boitSession, L"什么都没找到诶 再试试看？");
		}
		else
		{
			for (int i = 0; i < OIER_MAX_DISPLAY; i++)
			{
				if (PerPersonResult[i])
				{
					AddSizeVBuf(PerPersonResult[i], sizeof(WCHAR) * 1);
					((WCHAR*)(PerPersonResult[i]->Data))[(PerPersonResult[i]->Length / 2) - 1] = 0;
					SendBackMessage(boitSession, PerPersonResult[i]->Data);
				}
			}
			if (TotalResult > OIER_MAX_DISPLAY)
			{
				SendBackMessage(boitSession, L"更多信息请登录 OIerDB 网站详细查看");
			}

			cJSON_Delete(JsonRoot);
			for (int i = 0; i < OIER_MAX_DISPLAY; i++)
			{
				if (PerPersonResult[i])FreeVBuf(PerPersonResult[i]);
			}
		}


		switch (rand() % 8)
		{
		case 0:
			SendBackMessage(boitSession, L"orz nocriz!!!");
			break;
		case 1:
			SendBackMessage(boitSession, L"感谢 nocriz 开发的 OIerDB!");
			break;
		}
	}

}


