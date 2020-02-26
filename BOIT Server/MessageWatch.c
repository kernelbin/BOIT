#include<Windows.h>
#include"MessageWatch.h"
#include<process.h>




unsigned __stdcall MsgWatchTimerThread(void* Args);

void __stdcall MsgWatchTimerCallback(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
);

long long MsgWatchAllocID;

HANDLE hMsgWatchTimerThread; //消息监视计时器线程句柄

HANDLE hEventMsgWatchCleaning; //清理事件

HANDLE hEventPassArgStart; //给计时器线程传参开始

HANDLE hEventPassArgEnd; //给计时器传参完毕

SRWLOCK PassArgLock; // 作用域 ***************************
long long ArgAllocID;		// In
long long ArgTimeOut;		// In
HANDLE ArgWaitableTimer;	// Out
//********************************************************



int InitializeMessageWatch()
{
	InitializeSRWLock(&MsgWatchChainLock);
	InitializeSRWLock(&PassArgLock);
	MsgWatchAllocID = 0;

	RootMsgWatch = 0;

	hEventMsgWatchCleaning = CreateEvent(0, TRUE, 0, 0);

	hEventPassArgStart = CreateEvent(0, 0, 0, 0);
	hEventPassArgEnd = CreateEvent(0, 0, 0, 0);

	//启动计时器线程
	hMsgWatchTimerThread = _beginthreadex(0, 0, MsgWatchTimerThread, 0, 0, 0);
	return 0;
}


int FinalizeMessageWatch()
{
	AcquireSRWLockExclusive(&MsgWatchChainLock);
	if (RootMsgWatch)
	{
		for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
		{
			free(pList);
		}
	}
	ReleaseSRWLockExclusive(&MsgWatchChainLock);
	RootMsgWatch = 0;

	SetEvent(hEventMsgWatchCleaning);
	WaitForSingleObject(hMsgWatchTimerThread, INFINITE);

	CloseHandle(hMsgWatchTimerThread);
	CloseHandle(hEventMsgWatchCleaning);
	CloseHandle(hEventPassArgStart);
	CloseHandle(hEventPassArgEnd);
	return 0;
}

int RegisterMessageWatch(int WatchType,
	long long TimeOutInterval,// -1代表无穷监视
	long long GroupID,
	long long QQID,
	int SubType,
	WCHAR* AnonymousName,
	MSGWATCH_CALLBACK CallbackFunc,
	PBYTE pData)
{
	pBOIT_MSGWATCH MsgWatch = malloc(sizeof(BOIT_MSGWATCH));
	ZeroMemory(MsgWatch, sizeof(BOIT_MSGWATCH));

	MsgWatch->GroupID = GroupID;
	MsgWatch->QQID = QQID;
	MsgWatch->WatchType = WatchType;
	MsgWatch->MsgWatchID = InterlockedIncrement64(&MsgWatchAllocID);
	MsgWatch->Callback = CallbackFunc;
	MsgWatch->pData = pData;

	AcquireSRWLockExclusive(&MsgWatchChainLock);
	__try
	{
		pBOIT_MSGWATCH OrgHead = RootMsgWatch;
		RootMsgWatch = MsgWatch;
		MsgWatch->Next = OrgHead;
		if (OrgHead)
		{
			OrgHead->Last = MsgWatch;
		}
	}
	__finally
	{
		ReleaseSRWLockExclusive(&MsgWatchChainLock);
	}

	if (TimeOutInterval != -1)
	{
		AcquireSRWLockExclusive(&PassArgLock);
		ArgTimeOut = TimeOutInterval;
		ArgAllocID = MsgWatch->MsgWatchID;

		SignalObjectAndWait(hEventPassArgStart, hEventPassArgEnd, INFINITE, 0);

		MsgWatch->hTimer = ArgWaitableTimer;
		ReleaseSRWLockExclusive(&PassArgLock);
	}
	

	return 0;
}



int RemoveMessageWatchFromListUnSafe(pBOIT_MSGWATCH MsgWatch)
{
	if (MsgWatch->Last)
	{
		MsgWatch->Last->Next = MsgWatch->Next;
	}
	else
	{
		RootMsgWatch = MsgWatch->Next;
	}

	if (MsgWatch->Next)
	{
		MsgWatch->Next->Last = MsgWatch->Last;
	}
	return 0;
}


int RemoveMessageWatch(pBOIT_MSGWATCH MsgWatch)
{
	AcquireSRWLockExclusive(&MsgWatchChainLock);
	__try
	{
		RemoveMessageWatchFromListUnSafe(MsgWatch);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&MsgWatchChainLock);
	}

	free(MsgWatch);
	return 0;
}


int RemoveMessageWatchByID(long long MsgWatchAllocID)
{
	AcquireSRWLockExclusive(&MsgWatchChainLock);
	__try
	{
		if (RootMsgWatch)
		{
			for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
			{
				if (pList->MsgWatchID == MsgWatchAllocID)
				{
					RemoveMessageWatchFromListUnSafe(pList);
					free(pList);
					__leave;
				}
			}
		}
	}
	__finally
	{
		ReleaseSRWLockExclusive(&MsgWatchChainLock);
	}
	
	return 0;
}


unsigned __stdcall MsgWatchTimerThread(void* Args)
{
	while (1)
	{
		HANDLE WaitHandleList[2] = { hEventMsgWatchCleaning , hEventPassArgStart };
		DWORD dwRet = WaitForMultipleObjectsEx(2, WaitHandleList, FALSE, INFINITE, TRUE);

		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			return 0;
		case WAIT_OBJECT_0 + 1:
		{
			HANDLE hTimer = CreateWaitableTimer(0, 0, 0);
			LARGE_INTEGER LiTimeOut;
			LiTimeOut.QuadPart = -ArgTimeOut;
			SetWaitableTimer(hTimer, &LiTimeOut, 0, MsgWatchTimerCallback, (LPVOID)ArgAllocID, FALSE);
			ArgWaitableTimer = hTimer;
			SetEvent(hEventPassArgEnd);
		}
		break;
		}
	}
	return 0;
}


void __stdcall MsgWatchTimerCallback(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
)
{
	MSGWATCH_CALLBACK CallbackFunc = 0;
	PBYTE pData = 0;
	AcquireSRWLockExclusive(&MsgWatchChainLock);
	if (RootMsgWatch)
	{
		for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
		{
			if (pList->MsgWatchID == lpArgToCompletionRoutine)
			{
				CallbackFunc = pList->Callback;
				pData = pList->pData;
			}
		}
	}
	//RemoveMessageWatchByID(lpArgToCompletionRoutine,);
	ReleaseSRWLockExclusive(&MsgWatchChainLock);

	if (CallbackFunc)
	{
		CallbackFunc(lpArgToCompletionRoutine, pData, 0);
	}
	return;
}