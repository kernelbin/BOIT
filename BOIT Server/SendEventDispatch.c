#include<Windows.h>
#include"Global.h"
#include"EstablishConn.h"
#include"BOITEventType.h"
#include<strsafe.h>

int InitSendEventDispatch()
{
	InitializeSRWLock(&SendLock);
	return 0;
}

int SendEventPrivateMsg(long long QQID, WCHAR * Msg)
{
	if (GetConnState() == 0)
	{
		printf("和CoolQ之间的连接已经断开，消息未发送");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_PRIVATE;
		StringCchCopyW(pSharedMemSend->u.PrivateMsg.Msg, BOIT_MAX_TEXTLEN, Msg);
		pSharedMemSend->u.PrivateMsg.QQID = QQID;
		SetEvent(hEventSendStart);
		
		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//成功
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventGroupMsg(long long GroupID, WCHAR* Msg)
{
	if (GetConnState() == 0)
	{
		printf("和CoolQ之间的连接已经断开，消息未发送");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GROUP;
		StringCchCopyW(pSharedMemSend->u.GroupMsg.Msg, BOIT_MAX_TEXTLEN, Msg);
		pSharedMemSend->u.GroupMsg.GroupID = GroupID;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//成功
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}