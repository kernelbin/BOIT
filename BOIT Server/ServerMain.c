#include<Windows.h>
#include<stdio.h>
#include"EstablishConn.h"

int main()
{
	printf("BOIT ServerÕýÔÚÆô¶¯");
	InitEstablishConn();
	TryEstablishConn();

	while (1)
	{
		ConnWaitForObject(hEventRecvStart);

		MessageBoxW(0, pSharedMemRecv->u.PrivateMsg.Msg, L"qwq", 0);

		SetEvent(hEventRecvEnd);
	}


	return 0;
}