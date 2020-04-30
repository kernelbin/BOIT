#pragma once
#include<Windows.h>
#include"SessionManage.h"
#include"SharedMemStruct.h"


int SendPrivateMessage(long long QQID, WCHAR* Msg);
int RecvPrivateMessage(long long QQID, int SubType, WCHAR* Msg);

int SendGroupMessage(long long GroupID, WCHAR* Msg);
int RecvGroupMessage(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);

int RetrieveGroupMemberInfo(pBOIT_SESSION boitSession, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo);
int RetrieveStrangerInfo(pBOIT_SESSION boitSession, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo);

int SendBackMessage(pBOIT_SESSION boitSession, WCHAR* Msg);


