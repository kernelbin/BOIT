#include<Windows.h>
#include"Global.h"
#include"CQAPITransfer.h"
#include"EstablishConn.h"
#include"BOITEventType.h"

//生命周期

int AppInitialize()//初始化时会被执行
{
	//InitializeEventDispatch();
	InitializeSRWLock(&RecvLock);

	_beginthreadex(0, 0, WaitForConnThread, 0, 0, 0);
	return 0;
}

int AppFinialize()//结束时会被执行
{
	CleanConn();
	return 0;
}

int AppEnabled()//启用时执行（如果初始化时是启用的，会在AppInitialize后执行一次）
{
	return 0;
}

int AppDisabled()//禁用时执行（如果结束时是启用的，会在AppFinialize前执行一次，这点和cq原生函数不一样）
{
	return 0;
}



int HandlePrivateMessage(int subType, int msgId, long long fromQQ, const char* msg, int font)
{
	if (GetConnState() == 0)
	{
		AddLog(CQLOG_INFO, "BOIT", "和Server之间的连接已经断开，消息未处理");
		return 0;
	}
	AcquireSRWLockExclusive(&RecvLock);

	__try
	{
		pSharedMemRecv->EventType = BOIT_EVENT_RECV_PRIVATE;
		int cchWideCharLen = MultiByteToWideChar(CP_ACP, 0, msg, -1, 0, 0);
		cchWideCharLen = min(cchWideCharLen, BOIT_MAX_TEXTLEN);
		MultiByteToWideChar(CP_ACP, 0, msg, -1, pSharedMemRecv->u.PrivateMsg.Msg, cchWideCharLen);
		pSharedMemRecv->u.PrivateMsg.Msg[cchWideCharLen] = 0;
		pSharedMemRecv->u.PrivateMsg.Msg[cchWideCharLen + 1] = 0;

		pSharedMemRecv->u.PrivateMsg.QQID = fromQQ;

		SetEvent(hEventRecvStart);//等待对方读取

		if (ConnWaitForObject(hEventRecvEnd) == 0)
		{
			__leave;
		}
		//成功
	}
	__finally
	{
		ReleaseSRWLockExclusive(&RecvLock);
	}
	
	
	return 0;
}


int HandleGroupMessage(int subType, int msgId, long long fromGroup, long long fromQQ, const char * fromAnonymous, const char * msg, int font)
{
	if (GetConnState() == 0)
	{
		AddLog(CQLOG_INFO, "BOIT", "和Server之间的连接已经断开，消息未处理");
		return 0;
	}

	if (msg[0] == '#')
	{
		SendGroupMessage(fromGroup, "正在施工中\nGithub地址:https://github.com/kernelbin/BOIT \n快来star！\n有什么好的idea也欢迎来提issue！qwq我会认真看的");
	}
	return 0;
}
