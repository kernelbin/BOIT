#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include"CoolQDef.h"
#include"CoolQ_State.h"
#include "CQAPITransfer.h"

int SendPrivateMessage(long long QQID, const char* msg)
{
	return CQ_sendPrivateMsg(CQAuthCode, QQID, msg);
}


int SendGroupMessage(long long GroupID, const char* msg)
{
	return CQ_sendGroupMsg(CQAuthCode, GroupID, msg);
}


//const char * GetGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache)
//{
//	return CQ_getGroupMemberInfoV2(CQAuthCode, GroupID, QQID, NoCache);
//}
//
//
//const char* GetStrangerInfo(long long QQID, BOOL NoCache)
//{
//	return CQ_getStrangerInfo(CQAuthCode, QQID, NoCache);
//}