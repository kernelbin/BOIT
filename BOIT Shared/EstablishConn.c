#include<Windows.h>
#include"ObjectName.h"
#include"EstablishConn.h"
#include"SharedMemStruct.h"
#include<process.h>

//这个文件的代码是共享的，用于建立DLL和Server之间的连接


#define JUDGE_OBJ(Obj, iRet) \
if(Obj){\
if(GetLastError() == ERROR_ALREADY_EXISTS){\
if(iRet == -2 || iRet == 1){iRet = 1;}else{__leave;}\
}\
else{\
if(iRet == -2 || iRet == 0){iRet = 0;}else{__leave;}\
}\
}\
else{\
__leave;\
}


#define CloseHandleIf(x) if(x)CloseHandle(x);x=NULL;


unsigned __stdcall ProcessWatchThread(void* Args);

int InitEstablishConn()
{
	ConnState = 0;
	InitializeSRWLock(&ConnStateLock);
	
	return 0;
}

int TryEstablishConn()//return -1代表失败 0代表成功创建对象并等待到连接 1代表成功连接
{
	int iRet = -2;
	InitConnVar();
	__try
	{
		hEventConnect = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_CONNECT));
		JUDGE_OBJ(hEventConnect, iRet);

		hEventRecvStart = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_RECVSTART));
		JUDGE_OBJ(hEventRecvStart, iRet);

		hEventRecvEnd = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_RECVEND));
		JUDGE_OBJ(hEventRecvEnd, iRet);

		hEventSendStart = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_SENDSTART));
		JUDGE_OBJ(hEventSendStart, iRet);

		hEventSendEnd = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_SENDEND));
		JUDGE_OBJ(hEventSendEnd, iRet);

		hSharedMemProcess = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(SHARED_PROCESS_INFO), GET_OBJ_NAME(SHAREDMEM_RECV));
		JUDGE_OBJ(hSharedMemProcess, iRet);
		pSharedMemProcess = MapViewOfFile(hSharedMemProcess, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_PROCESS_INFO));

		hSharedMemRecv = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(EVENT_RECV), GET_OBJ_NAME(SHAREDMEM_RECV));
		JUDGE_OBJ(hSharedMemRecv, iRet);
		pSharedMemRecv = MapViewOfFile(hSharedMemRecv, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(EVENT_RECV));

		hSharedMemSend = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(EVENT_SEND), GET_OBJ_NAME(SHAREDMEM_RECV));
		JUDGE_OBJ(hSharedMemSend, iRet);
		pSharedMemSend = MapViewOfFile(hSharedMemSend, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(EVENT_SEND));

	}
	__finally
	{
		switch(iRet)
		{
		case -1:
			CleanConn();
			break;
		case 0:
			pSharedMemProcess->pid[0] = GetCurrentProcessId();
			WaitForSingleObject(hEventConnect, INFINITE);
			break;
		case 1:
			//把自己的进程句柄写入
			if (pSharedMemProcess->pid[0] == 0)
			{
				pSharedMemProcess->pid[0] = GetCurrentProcessId();
			}
			else
			{
				pSharedMemProcess->pid[1] = GetCurrentProcessId();
			}
			SetEvent(hEventConnect);
			break;
		}
		
	}
	
	if (pSharedMemProcess->pid[0] == GetCurrentProcessId())
	{
		hOtherSideProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pSharedMemProcess->pid[1]);
	}
	else
	{
		hOtherSideProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pSharedMemProcess->pid[0]);
	}
	_beginthreadex(0, 0, ProcessWatchThread, 0, 0, 0);

	AcquireSRWLockExclusive(&ConnStateLock);
	ConnState = 1;
	ReleaseSRWLockExclusive(&ConnStateLock);
	MessageBox(NULL, TEXT("Conn!"), TEXT("Conn!"), 0);
	return iRet;
}

int TryReConn()
{
	WaitForSingleObject(hEventConnect, INFINITE);

	if (pSharedMemProcess->pid[0] == GetCurrentProcessId())
	{
		hOtherSideProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pSharedMemProcess->pid[1]);
	}
	else
	{
		hOtherSideProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pSharedMemProcess->pid[0]);
	}
	_beginthreadex(0, 0, ProcessWatchThread, 0, 0, 0);

	AcquireSRWLockExclusive(&ConnStateLock);
	ConnState = 1;
	ReleaseSRWLockExclusive(&ConnStateLock);

	MessageBox(NULL, TEXT("REConn!"), TEXT("REConn!"), 0);
	return 0;
}

BOOL CleanConn()
{
	CloseHandleIf(hEventConnect);
	
	CloseHandleIf(hEventRecvStart);

	CloseHandleIf(hEventRecvEnd);

	CloseHandleIf(hEventSendStart);

	CloseHandleIf(hEventSendEnd);

	if (pSharedMemProcess)
	{
		UnmapViewOfFile(pSharedMemProcess);
	}
	CloseHandleIf(hSharedMemProcess);

	if (pSharedMemRecv)
	{
		UnmapViewOfFile(pSharedMemRecv);
	}
	CloseHandleIf(hSharedMemRecv);

	if (pSharedMemSend)
	{
		UnmapViewOfFile(pSharedMemSend);
	}
	CloseHandleIf(hSharedMemSend);

	return 0;
}

BOOL InitConnVar()
{
	hEventConnect = NULL;

	hEventRecvStart = NULL;

	hEventRecvEnd = NULL;

	hEventSendStart = NULL;

	hEventSendEnd = NULL;

	pSharedMemProcess = NULL;

	hSharedMemProcess = NULL;

	pSharedMemRecv = NULL;

	hSharedMemRecv = NULL;

	pSharedMemSend = NULL;
	
	hSharedMemSend = NULL;

	return 0;
}

unsigned __stdcall ProcessWatchThread(void* Args)
{
	//监视对方死亡
	while (1)
	{
		WaitForSingleObject(hOtherSideProcess, INFINITE);
		//擦除信息
		if (pSharedMemProcess->pid[0] == GetCurrentProcessId())
		{
			pSharedMemProcess->pid[1] = 0;
		}
		else
		{
			pSharedMemProcess->pid[0] = 0;
		}
		hOtherSideProcess = 0;

		AcquireSRWLockExclusive(&ConnStateLock);
		ConnState = 0;
		ReleaseSRWLockExclusive(&ConnStateLock);

		TryReConn();
	}
	
	return 0;
}

int GetConnState()
{
	BOOL bRet;
	AcquireSRWLockExclusive(&ConnStateLock);
	bRet = ConnState;
	ReleaseSRWLockExclusive(&ConnStateLock);
	return bRet;
}