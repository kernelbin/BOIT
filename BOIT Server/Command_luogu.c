#include<Windows.h>
#include<stdlib.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"
#include"AsyncINet.h"
#include"cJSON.h"
#include"EncodeConvert.h"

pASYNCINET_INFO LuoguInetInfo;
WCHAR LuoguServerName[] = L"www.luogu.com.cn";

int CmdMsg_luogu_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
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
		SendBackMessage(boitSession, L"输入#luogu help查看帮助");
		return 0;
	}

	WCHAR* OrderStr[] = { L"help",L"profile",L"problem" };
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
		SendBackMessage(boitSession, L"查询有关luogu的信息。\n#luogu profile [用户名] 以查询profile\n#luogu problem 以查询题目信息\n#luogu help 以查看该帮助信息");

		break;
	case 1:
	{
		SendBackMessage(boitSession, Corpus_FunctionDeving());
	}

	break;
	case 2:
	{

		if ((GetBOITSessionType(boitSession) == BOITSESS_TYPE_GROUP) &&
			CheckGroupToken(GetBOITSessionGroupID(boitSession), L"PrivilegeQueryLuoguProblem") == 0)
		{
			SendBackMessage(boitSession, L"该群禁止了查询洛谷题目。请在私聊中查询或联系管理员开放功能。");
			return 0;
		}

		WCHAR* OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
		OrderBase += GetCmdSpaceLen(OrderBase);
		int QueryStrlen = GetCmdParamLen(OrderBase);
		if (QueryStrlen >= 64)
		{
			SendBackMessage(boitSession, L"这是啥题目啊，ID这么长？");
		}
		else if (QueryStrlen == 0)
		{
			SendBackMessage(boitSession, L"你要查询的是哪个题目诶？（试试#luogu problem P1001）");
		}
		else
		{
			QueryLuoguProblemInfo(boitSession, OrderBase);
		}

	}

	break;
	default:
		SendBackMessage(boitSession, L"未找到指令。输入#luogu help查看帮助");
		break;
	}
	/*else
	{

	}*/

	return 0;
}


int CmdEvent_luogu_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_CMDLOAD:
		LuoguInetInfo = AsyncINetInit(LuoguServerName);
		break;

	case EC_CMDFREE:
		//等待线程结束
		AsyncINetCleanup(LuoguServerName);
		break;
	}
	return 0;
}


BOOL ParseLuoguProblemInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);
	BOOL bSuccess = FALSE;

	pVBUF ReplyStr = AllocVBuf();
	__try
	{
		if (!JsonRoot)
		{
			SendBackMessage(boitSession, L"貌似出了点小问题诶quq");
			__leave;
		}
		if (!JsonRoot->child)
		{
			SendBackMessage(boitSession, L"貌似出了点小问题诶quq");
			__leave;
		}

		cJSON* JsonStatusNode, * JsonDataNode;
		if (_stricmp(JsonRoot->child->string, "status"))
		{
			SendBackMessage(boitSession, L"貌似出了点小问题诶quq");
			__leave;
		}
		JsonStatusNode = JsonRoot->child;
		switch (JsonStatusNode->valueint)
		{
		case 200:
			break;
		case 404:
			SendBackMessage(boitSession, L"没有找到这道题目诶quq");
			__leave;
		case 500:
			SendBackMessage(boitSession, L"服务器出现了错误诶qwq");
			__leave;
		}
		if (_stricmp(JsonRoot->child->next->string, "data"))
		{
			SendBackMessage(boitSession, L"貌似出了点小问题诶quq");
			__leave;
		}
		JsonDataNode = JsonRoot->child->next;

		char* DataField[] = { "StringPID", "Name","Background","Description","InputFormat", "OutputFormat","Sample","Hint" };
		cJSON* JsonDataField[_countof(DataField)] = { 0 };

		for (cJSON* EnumField = JsonDataNode->child; EnumField; EnumField = EnumField->next)
		{
			for (int i = 0; i < _countof(DataField); i++)
			{
				if ((!JsonDataField[i]) && _strcmpi(EnumField->string, DataField[i]) == 0)
					JsonDataField[i] = EnumField;
			}
		}

		if ((!JsonDataField[0]) || (!(JsonDataField[0]->valuestring)) || (!JsonDataField[1]) || (!(JsonDataField[1]->valuestring)))
		{
			SendBackMessage(boitSession, L"貌似出了点小问题诶quq");
			__leave;
		}

		WCHAR* StrBuf;
		StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[0]->valuestring, -1, NULL);
		VBufferAppendStringW(ReplyStr, StrBuf);
		free(StrBuf);
		VBufferAppendStringW(ReplyStr, L": ");
		StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[1]->valuestring, -1, NULL);
		VBufferAppendStringW(ReplyStr, StrBuf);
		free(StrBuf);


		if (JsonDataField[2] && JsonDataField[2]->valuestring && strlen(JsonDataField[2]->valuestring))
		{
			VBufferAppendStringW(ReplyStr, L"\n题目背景：\n");
			StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[2]->valuestring, -1, NULL);
			VBufferAppendStringW(ReplyStr, StrBuf);
		}
		if (JsonDataField[3] && JsonDataField[3]->valuestring && strlen(JsonDataField[3]->valuestring))
		{
			VBufferAppendStringW(ReplyStr, L"\n题目描述：\n");
			StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[3]->valuestring, -1, NULL);
			VBufferAppendStringW(ReplyStr, StrBuf);
		}
		if (JsonDataField[4] && JsonDataField[4]->valuestring && strlen(JsonDataField[4]->valuestring))
		{
			VBufferAppendStringW(ReplyStr, L"\n输入格式：\n");
			StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[4]->valuestring, -1, NULL);
			VBufferAppendStringW(ReplyStr, StrBuf);
		}
		if (JsonDataField[5] && JsonDataField[5]->valuestring && strlen(JsonDataField[5]->valuestring))
		{
			VBufferAppendStringW(ReplyStr, L"\n输出格式：\n");
			StrBuf = StrConvMB2WC(CP_UTF8, JsonDataField[5]->valuestring, -1, NULL);
			VBufferAppendStringW(ReplyStr, StrBuf);
		}
		bSuccess = TRUE;
	}
	__finally
	{

		if (bSuccess)
		{
			AddSizeVBuf(ReplyStr, sizeof(WCHAR) * 1);
			((WCHAR*)(ReplyStr->Data))[(ReplyStr->Length / 2) - 1] = 0;
			SendBackMessage(boitSession, ReplyStr->Data);
		}
		if (JsonRoot)
		{
			cJSON_Delete(JsonRoot);
		}
	}

}

int AsyncLuoguProblemInfoCallback(
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
		ParseLuoguProblemInfoJsonAndSend((pBOIT_SESSION)ExtData, ReceivedBuf->Data);

		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"哎呀，查询Luogu题目失败了");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)ExtData);
	return 0;
}

BOOL QueryLuoguProblemInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
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

	UrlBuffer = malloc((wcslen(L"/api/problem/detail/") + WCSLen + 1) * sizeof(WCHAR));
	swprintf_s(UrlBuffer, (wcslen(L"/api/problem/detail/") + WCSLen + 1), L"/api/problem/detail/%ls", WCEncodedSearch);
	free(WCEncodedSearch);


	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(LuoguInetInfo, UrlBuffer, newBoitSess, AsyncLuoguProblemInfoCallback);


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
	/*pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(LuoguInetInfo, L"/api/problem/detail/P1001", newBoitSess, AsyncLuoguProblemInfoCallback);*/
}

