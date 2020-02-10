#include<Windows.h>
#include "RecvEventHandler.h"
#include"EstablishConn.h"
#include"Global.h"
#include"BOITEventType.h"
#include<strsafe.h>
#include"APITransfer.h"
unsigned __stdcall RecvEventThread();


int StartRecvEventHandler()
{
	SYSTEM_INFO SysInfo;//用来取得CPU数量等信息
	GetSystemInfo(&SysInfo);

	for (unsigned int i = 0; i < SysInfo.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL, 0, RecvEventThread, (LPVOID)0, 0, NULL);
	}
	return 0;
}


unsigned __stdcall RecvEventThread()
{
	while (1)
	{
		while (GetConnState() == 1)
		{
			if (ConnWaitForObject(hEventRecvStart) == 0)
			{
				break;
			}
			EVENT_RECV RecvEvent;
			//TODO: 复制对象，分发任务
			switch (pSharedMemRecv->EventType)
			{
			case BOIT_EVENT_RECV_PRIVATE:
				RecvEvent.EventType = BOIT_EVENT_RECV_PRIVATE;
				RecvEvent.u.PrivateMsg.QQID = pSharedMemRecv->u.PrivateMsg.QQID;
				StringCchCopyW(RecvEvent.u.PrivateMsg.Msg, BOIT_MAX_TEXTLEN, pSharedMemRecv->u.PrivateMsg.Msg);

				break;
			case BOIT_EVENT_RECV_GROUP:
				RecvEvent.EventType = BOIT_EVENT_RECV_GROUP;
				RecvEvent.u.GroupMsg.QQID = pSharedMemRecv->u.GroupMsg.QQID;
				RecvEvent.u.GroupMsg.GroupID = pSharedMemRecv->u.GroupMsg.GroupID;
				StringCchCopyW(RecvEvent.u.GroupMsg.Msg, BOIT_MAX_TEXTLEN, pSharedMemRecv->u.GroupMsg.Msg);
				StringCchCopyW(RecvEvent.u.GroupMsg.AnonymousName, BOIT_MAX_NICKLEN, pSharedMemRecv->u.GroupMsg.AnonymousName);

				break;
			default:
				break;
			}
			SetEvent(hEventRecvEnd);
			//分发
			switch (pSharedMemRecv->EventType)
			{
			case BOIT_EVENT_RECV_PRIVATE:
				RecvPrivateMessage(pSharedMemRecv->u.PrivateMsg.QQID,
					pSharedMemRecv->u.PrivateMsg.Msg);
				break;
			case BOIT_EVENT_RECV_GROUP:
				RecvGroupMessage(pSharedMemRecv->u.GroupMsg.GroupID,
					pSharedMemRecv->u.GroupMsg.QQID,
					pSharedMemRecv->u.GroupMsg.AnonymousName,
					pSharedMemRecv->u.GroupMsg.Msg);
				break;
			}
		}
		Sleep(1000);
	}
}





