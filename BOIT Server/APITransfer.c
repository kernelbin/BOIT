#include<Windows.h>
#include"RecvEventHandler.h"
#include "APITransfer.h"
#include"SendEventDispatch.h"
#include"CommandManager.h"

//从这里开始消息流正式进入应用层

int SendPrivateMessage(long long QQID, WCHAR * Msg)
{
	SendEventPrivateMsg(QQID, Msg);
	return 0;
}

int RecvPrivateMessage(long long QQID, int SubType, WCHAR* Msg)
{
	int PrefixLen;
	if (CheckIsCommand(Msg, &PrefixLen))
	{
		CommandHandler(0, QQID, SubType, 0, Msg + PrefixLen);
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
	int PrefixLen;
	if (CheckIsCommand(Msg, &PrefixLen))
	{
		CommandHandler(GroupID, QQID, SubType, AnonymousName, Msg + PrefixLen);
	}
	return 0;
}

int RetrieveGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo)
{
	return SendEventGetGroupMemberInfo(GroupID, QQID, NoCache, GroupMemberInfo);
}

int RetrieveStrangerInfo(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo)
{
	return SendEventGetStrangerInfo(QQID, NoCache, StrangerInfo);
}





int SendBackMessage(long long GroupID, long long QQID, WCHAR* Msg)
{
	if (GroupID)
	{
		SendGroupMessage(GroupID, Msg);
	}
	else
	{
		SendPrivateMessage(QQID, Msg);
	}
	return 0;
}
