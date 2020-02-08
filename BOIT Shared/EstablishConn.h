#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


HANDLE hEventConnect;
HANDLE hEventDeconn;

HANDLE hEventRecvStart;//开始接收 消息/事件 。
HANDLE hEventRecvEnd;//接收 消息/事件 结束。

HANDLE hEventSendStart;//开始接收 消息/事件 。
HANDLE hEventSendEnd;//接收 消息/事件 结束。

HANDLE hSharedMemProcess;//Server 和 dll 相关进程信息
HANDLE hSharedMemRecv;
HANDLE hSharedMemSend;

HANDLE hOtherSideProcess;

pSHARED_PROCESS_INFO pSharedMemProcess;
pEVENT_RECV pSharedMemRecv;
pEVENT_SEND pSharedMemSend;

SRWLOCK ConnStateLock;
BOOL ConnState; //0表示没连接，1表示连接


int InitEstablishConn();
int TryEstablishConn();
int GetConnState();
BOOL CleanConn();
BOOL InitConnVar();

BOOL ConnWaitForObject(HANDLE hObject);