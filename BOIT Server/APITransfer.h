#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


int SendPrivateMessage(long long QQID, WCHAR* Msg);
int RecvPrivateMessage(long long QQID, int SubType, WCHAR* Msg);

int SendGroupMessage(long long GroupID, WCHAR* Msg);
int RecvGroupMessage(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);

BOOL RetrieveGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo);
BOOL RetrieveStrangerInfo(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo);

int SendBackMessage(long long GroupID, long long QQID, WCHAR* Msg);


