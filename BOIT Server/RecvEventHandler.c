#include<Windows.h>
#include "RecvEventHandler.h"
#include"EstablishConn.h"
#include"Global.h"

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
			pEVENT_RECV RecvEvent;
			//TODO: 复制对象，分发任务
			MessageBoxW(0, pSharedMemRecv->u.PrivateMsg.Msg, L"qwq", 0);
			SendEventPrivateMsg(pSharedMemRecv->u.PrivateMsg.QQID, L"nihao");
			SetEvent(hEventRecvEnd);
		}
		Sleep(1000);
	}
}

