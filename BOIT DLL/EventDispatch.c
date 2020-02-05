#include<Windows.h>
#include"ObjectName.h"
#include"EstablishConn.h"
#include<Process.h>
#include "EventDispatch.h"
//接受来自CQ的各种事件，并且转发

SRWLOCK SendLock;
SRWLOCK RecvLock;



unsigned __stdcall WaitForConnThread(void* Args);

int InitializeEventDispatch()
{
	InitializeSRWLock(&SendLock);
	InitializeSRWLock(&RecvLock);

	_beginthreadex(0, 0, WaitForConnThread, 0, 0, 0);
	return 0;
}

int FinalizeEventDispatch()
{

	return 0;
}

unsigned __stdcall WaitForConnThread(void *Args)

{
	TryEstablishConn();
	return 0;
}
