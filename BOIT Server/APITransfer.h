#pragma once
#include<Windows.h>

int SendPrivateMessage(long long QQID, WCHAR* Msg);
int RecvPrivateMessage(long long QQID, WCHAR* Msg);

int SendGroupMessage(long long GroupID, WCHAR* Msg);
int RecvGroupMessage(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);


