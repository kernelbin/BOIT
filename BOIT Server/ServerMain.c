#include<Windows.h>
#include<stdio.h>
#include"EstablishConn.h"
#include"Global.h"
#include"RecvEventHandler.h"


int main()
{
	printf("BOIT Server正在启动\n");
	InitServerState();
	InitSendEventDispatch();;
	InitEstablishConn();
	TryEstablishConn();

	printf("连接成功！\n");
	
	StartRecvEventHandler();

	WaitForSingleObject(hEventServerStop, INFINITE);

	//TODO:清理工作

	return 0;
}