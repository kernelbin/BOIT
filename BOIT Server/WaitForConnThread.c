#include<Windows.h>
#include<process.h>
#include"EstablishConn.h"
#include"Global.h"



unsigned __stdcall WaitForConnThread(void* Args)
{
	InitEstablishConn();
	TryEstablishConn(NULL);
	StartSendEventHandler();
	return 0;
}