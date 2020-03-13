#include<Windows.h>
#include"Global.h"
#include"EstablishConn.h"
#include"BOITEventType.h"
#include<strsafe.h>
#include "SendEventDispatch.h"
#include"SharedMemStruct.h"


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
		//读取返回值
		//printf("测试消息ID是%d\n", pSharedMemSend->u.PrivateMsg.iRet);

		SetEvent(hEventSendRet);

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
		//printf("测试消息ID是%d\n", pSharedMemSend->u.GroupMsg.iRet);

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventGetGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo)
{
	if (GetConnState() == 0)
	{
		printf("和CoolQ之间的连接已经断开，消息未发送");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GET_GROUPMEMBER_INFO;
		pSharedMemSend->u.GroupMemberInfo.GroupID = GroupID;
		pSharedMemSend->u.GroupMemberInfo.QQID = QQID;
		pSharedMemSend->u.GroupMemberInfo.NoCache = TRUE;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//成功
		memcpy(GroupMemberInfo, &(pSharedMemSend->u.GroupMemberInfo.GroupMemberInfo), sizeof(BOIT_GROUPMEMBER_INFO));
		

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventGetStrangerInfo(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo)
{
	if (GetConnState() == 0)
	{
		printf("和CoolQ之间的连接已经断开，消息未发送");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GET_STRANGER_INFO;
		pSharedMemSend->u.StrangerInfo.QQID = QQID;
		pSharedMemSend->u.StrangerInfo.NoCache = TRUE;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//成功
		memcpy(StrangerInfo, &(pSharedMemSend->u.StrangerInfo.StrangerInfo), sizeof(BOIT_STRANGER_INFO));

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventRetrieveCQPath(WCHAR * Path)
{
	if (GetConnState() == 0)
	{
		printf("和CoolQ之间的连接已经断开，消息未发送");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_RETRIEVE_CQPATH;
	
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//成功
		//memcpy(StrangerInfo, &(pSharedMemSend->u.StrangerInfo.StrangerInfo), sizeof(BOIT_STRANGER_INFO));
		wcscpy_s(Path,MAX_PATH, pSharedMemSend->u.CQPath.CQPath);
		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}



