#include<Windows.h>
#include"EstablishConn.h"
#include"CQAPITransfer.h"
#include"BOITEventType.h"
#include<process.h>

unsigned __stdcall SendEventThread(void* Args);

int StartSendEventHandler()
{
	_beginthreadex(NULL, 0, SendEventThread, (LPVOID)0, 0, NULL);
	
	return 0;
}


unsigned __stdcall SendEventThread(void *Args)
{
	while (1)
	{
		while (GetConnState() == 1)
		{
			if (ConnWaitForObject(hEventSendStart) == 0)
			{
				break;
			}
		
			//·ÖÀà£º
			switch (pSharedMemSend->EventType)
			{
			case BOIT_EVENT_SEND_PRIVATE:
			{
				char* ToSendText;
				int ccbLen;
				ccbLen = WideCharToMultiByte(54936, 0, pSharedMemSend->u.PrivateMsg.Msg, -1, 0, 0, 0, 0);
				ToSendText = malloc(sizeof(char) * (ccbLen + 1));
				WideCharToMultiByte(54936, 0, pSharedMemSend->u.PrivateMsg.Msg, -1, ToSendText, ccbLen+1, 0, 0);
				int iRet = SendPrivateMessage(pSharedMemSend->u.PrivateMsg.QQID, ToSendText);
				free(ToSendText);
				pSharedMemSend->u.PrivateMsg.iRet = iRet;
			}
				break;
			case BOIT_EVENT_SEND_GROUP:
			{
				char* ToSendText;
				int ccbLen;
				ccbLen = WideCharToMultiByte(54936, 0, pSharedMemSend->u.GroupMsg.Msg, -1, 0, 0, 0, 0);
				ToSendText = malloc(sizeof(char) * (ccbLen + 1));
				WideCharToMultiByte(54936, 0, pSharedMemSend->u.GroupMsg.Msg, -1, ToSendText, (ccbLen + 1), 0, 0);
				int iRet = SendGroupMessage(pSharedMemSend->u.GroupMsg.GroupID, ToSendText);
				free(ToSendText);
				pSharedMemSend->u.GroupMsg.iRet = iRet;
			}
				break;
			}
			
			SetEvent(hEventSendEnd);

			if (ConnWaitForObject(hEventSendRet) == 0)
			{
				break;
			}
		}
		Sleep(1000);
	}
	
}

