#include<Windows.h>
#include"RecvEventHandler.h"
#include "APITransfer.h"
#include"SendEventDispatch.h"
#include"CommandManager.h"
#include"MessageWatch.h"
#include"SessionManage.h"
//从这里开始消息流正式进入应用层

int SendPrivateMessage(long long QQID, WCHAR * Msg)
{
	SendEventPrivateMsg(QQID, Msg);
	return 0;
}

int RecvPrivateMessage(long long QQID, int SubType, WCHAR* Msg)
{
	pBOIT_SESSION boitSession = InitBOITSession(0, QQID, 0, SubType);
	if (!boitSession)
	{
		return 0;
	}
	__try
	{
		if (MessageWatchFilter(0, QQID, SubType, 0, Msg) == BOIT_MSGWATCH_BLOCK)
		{
			__leave;
		}
		int PrefixLen;
		if (CheckIsCommand(Msg, &PrefixLen))
		{
			CommandHandler(boitSession, Msg + PrefixLen);
		}
		__leave;
	}
	__finally
	{
		FreeBOITSession(boitSession);
	}
	return 0;
}

int SendGroupMessage(long long GroupID, WCHAR* Msg)
{
	SendEventGroupMsg(GroupID, Msg);
	return 0;
}

int RecvGroupMessage(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	pBOIT_SESSION boitSession = InitBOITSession(GroupID, QQID, AnonymousName, SubType);
	if (!boitSession)
	{
		return 0;
	}
	__try
	{
		if (MessageWatchFilter(GroupID, QQID, SubType, AnonymousName, Msg) == BOIT_MSGWATCH_BLOCK)
		{
			__leave;
		}
		int PrefixLen;
		if (CheckIsCommand(Msg, &PrefixLen))
		{
			CommandHandler(boitSession, Msg + PrefixLen);
		}
		__leave;
	}
	__finally
	{
		FreeBOITSession(boitSession);
	}
	return 0;
}

int RetrieveGroupMemberInfo(pBOIT_SESSION boitSession, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo)
{
	return SendEventGetGroupMemberInfo(GetBOITSessionGroupID(boitSession),
		GetBOITSessionQQID(boitSession),
		NoCache,
		GroupMemberInfo);
}

int RetrieveStrangerInfo(pBOIT_SESSION boitSession, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo)
{
	return SendEventGetStrangerInfo(GetBOITSessionQQID(boitSession),
		NoCache,
		StrangerInfo);
}





int SendBackMessage(pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if (boitSession->GroupID)
	{
		SendGroupMessage(GetBOITSessionGroupID(boitSession), Msg);
	}
	else
	{
		SendPrivateMessage(GetBOITSessionQQID(boitSession), Msg);
	}
	return 0;
}
