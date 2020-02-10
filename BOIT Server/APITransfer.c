#include<Windows.h>
#include"RecvEventHandler.h"
#include "APITransfer.h"
#include"SendEventDispatch.h"

//从这里开始消息流正式进入应用层

int SendPrivateMessage(long long QQID, WCHAR * Msg)
{
	SendEventPrivateMsg(QQID, Msg);
	return 0;
}

int RecvPrivateMessage(long long QQID, WCHAR* Msg)
{
	SendGroupMessage(217886986, L"收到一条私聊消息");
	SendGroupMessage(217886986, Msg);
	return 0;
}

int SendGroupMessage(long long GroupID, WCHAR* Msg)
{
	SendEventGroupMsg(GroupID, Msg);
	return 0;
}

int RecvGroupMessage(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	SendPrivateMessage(1160386205, L"收到一条群聊消息");
	SendPrivateMessage(1160386205, Msg);
	return 0;
}
