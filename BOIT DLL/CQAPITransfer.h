#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include"CoolQDef.h"
#include"CoolQ_State.h"

int SendPrivateMessage(long long QQID, const char* msg);

int SendGroupMessage(long long GroupID, const char* msg);

char* GetGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache);

char* GetStrangerInfo(long long QQID, BOOL NoCache);
