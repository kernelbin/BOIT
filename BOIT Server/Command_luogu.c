#include<Windows.h>
#include<stdlib.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"


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

	WCHAR* OrderStr[] = { L"help",L"profile",L"problem"};
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
		SendBackMessage(boitSession, Corpus_FunctionDeving());
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
