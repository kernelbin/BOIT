#include<Windows.h>
#include"SharedMemStruct.h"

int SendEventPrivateMsg(long long QQID, WCHAR* Msg);

int SendEventGroupMsg(long long GroupID, WCHAR* Msg);

int SendEventGetGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo);

int SendEventGetStrangerInfo(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo);

int SendEventRetrieveCQPath(WCHAR* Path);
