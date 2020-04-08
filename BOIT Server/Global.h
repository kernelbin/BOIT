#pragma once
#include<Windows.h>

SRWLOCK SendLock;

HANDLE hEventServerStop;

BOOL bBOITRemove; //是否卸载程序

int InitServerState();
int ServerStop();

int StartSendEventHandler();

int InitSendEventDispatch();


//消息回复
BOOL InitMessageReply();
