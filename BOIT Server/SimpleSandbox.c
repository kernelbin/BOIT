#include<Windows.h>
#include<process.h>
#include<sddl.h>
#include "SimpleSandbox.h"

#define COMPPORT_MAX_THREAD 16



HANDLE hJobObjPortThread[COMPPORT_MAX_THREAD];//线程句柄表
HANDLE hPipeIOPortThread[COMPPORT_MAX_THREAD];//线程句柄表

int g_JobObjCompThreadNum;
int g_PipeIOCompThreadNum;
SRWLOCK SandboxListLock;

unsigned __stdcall JobObjCompletionPort(LPVOID Args);
unsigned __stdcall PipeIOCompletionPort(LPVOID Args);
unsigned __stdcall TerminateJobTimerThread(LPVOID Args);

#define JOB_OBJECT_MSG_MY_CLEANUP 128 //瞎写的数字，比JobObj的IOCP最大消息号大就OK


HANDLE JobObjCompPort = 0;
HANDLE PipeIOCompPort = 0;


LONGLONG AllocSandboxID = 0;


//线程间传参数用
HANDLE TimerThread;

SRWLOCK PassArgTimerLock;//作用域*************
LONGLONG g_ArgTimeLimit; // In
LONGLONG g_ArgSandboxID; // In
HANDLE hWaitableTimer;   // Out
//********************************************
HANDLE EventPassArgStart;
HANDLE EventPassArgEnd;


HANDLE SandboxCleaning;


BOOL CreateLimitedProcessW(
	_In_opt_ WCHAR lpApplicationName[],
	_Inout_opt_ WCHAR lpCommandLine[],
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles,
	_In_ DWORD dwCreationFlags,
	_In_opt_ LPVOID lpEnvironment,
	_In_opt_ WCHAR lpCurrentDirectory[],
	_In_ LPSTARTUPINFOW lpStartupInfo,
	_Out_ LPPROCESS_INFORMATION lpProcessInformation);

void __stdcall TerminateJobTimerRoutine(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
);


BOOL CreateOverlappedNamedPipePair(PHANDLE hReadPipe, PHANDLE hWritePipe, DWORD nSize);



int InitializeSandbox(int JobObjCompThreadNum, int PipeIOCompThreadNum)
{
	//创建一个完成端口，启动线程处理。
	//创建两个线程分别用于Pipe Read/Write
	InitializeSRWLock(&SandboxListLock);


	RootSandbox = 0;

	//指示是否沙盒正在关闭
	SandboxCleaning = CreateEvent(0, TRUE, 0, 0);


	if (JobObjCompThreadNum == 0 || PipeIOCompThreadNum == 0)
	{
		SYSTEM_INFO SysInfo;//用来取得CPU数量等信息
		GetSystemInfo(&SysInfo);
		JobObjCompThreadNum = (JobObjCompThreadNum == 0) ? SysInfo.dwNumberOfProcessors : JobObjCompThreadNum;
		PipeIOCompThreadNum = (PipeIOCompThreadNum == 0) ? SysInfo.dwNumberOfProcessors : PipeIOCompThreadNum;
	}
	g_JobObjCompThreadNum = JobObjCompThreadNum;
	g_PipeIOCompThreadNum = PipeIOCompThreadNum;

	if (JobObjCompThreadNum > COMPPORT_MAX_THREAD || PipeIOCompThreadNum > COMPPORT_MAX_THREAD)
	{
		return 0;
	}

	JobObjCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, JobObjCompThreadNum);

	PipeIOCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, PipeIOCompThreadNum);

	for (int i = 0; i < (int)JobObjCompThreadNum; i++)
	{
		hJobObjPortThread[i] = (HANDLE)_beginthreadex(NULL, 0, JobObjCompletionPort, (LPVOID)0, 0, NULL);
	}
	for (int i = 0; i < (int)PipeIOCompThreadNum; i++)
	{
		hPipeIOPortThread[i] = (HANDLE)_beginthreadex(NULL, 0, PipeIOCompletionPort, (LPVOID)0, 0, NULL);
	}


	//初始化计时器线程
	InitializeSRWLock(&PassArgTimerLock);
	EventPassArgStart = CreateEvent(0, 0, 0, 0);
	EventPassArgEnd = CreateEvent(0, 0, 0, 0);

	TimerThread = (HANDLE)_beginthreadex(NULL, 0, TerminateJobTimerThread, (LPVOID)0, 0, NULL);
	/*我需要：
		维护SESSION和Job之间的关系，SESSION与IO线程之间的关系
		Job对象可传递一个Key (在Assoc的时候指定)
		IO线程可以利用Overlapped的特性进行传参
		WriteFileEx是一次性的。只要在完成端口校验一下是否完成输入就好了。（不排除还没输入进程就死了，试试）
		ReadFileEx可能比较烦人*/
	return 0;
}


int FinalizeSandbox()
{
	SetEvent(SandboxCleaning);//沙盒关闭ing
	
	AcquireSRWLockExclusive(&SandboxListLock);

	if (RootSandbox)
	{
		for (pSANDBOX_CHAIN pList = RootSandbox; pList; pList = pList->NextSandbox)
		{
			TerminateJobObject(pList->hJob, 1); //全部终结
		}
	}
	ReleaseSRWLockExclusive(&SandboxListLock);

	for (int i = 0; i < g_JobObjCompThreadNum; i++)
	{
		//PostQueuedCompletionStatus要求传递一个Overlapped结构，我是真的不想传！！！
		//保险起见还是传递一下吧万一UB了呢，在完成端口那边再释放
		LPOVERLAPPED lpOverlapped = malloc(sizeof(OVERLAPPED));
		ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));

		PostQueuedCompletionStatus(JobObjCompPort, JOB_OBJECT_MSG_MY_CLEANUP, 0, lpOverlapped);
	}

	for (int i = 0; i < g_PipeIOCompThreadNum; i++)
	{
		PipeIOSendClose(PipeIOCompPort);
	}

	WaitForMultipleObjects(g_JobObjCompThreadNum, hJobObjPortThread, 1, INFINITE);
	WaitForMultipleObjects(g_PipeIOCompThreadNum, hPipeIOPortThread, 1, INFINITE);
	WaitForSingleObject(TimerThread, INFINITE);

	CloseHandle(EventPassArgStart);
	CloseHandle(EventPassArgEnd);

	CloseHandle(JobObjCompPort);
	CloseHandle(PipeIOCompPort);

	CloseHandle(SandboxCleaning);
	return 0;
}


pSANDBOX CreateSimpleSandboxW(WCHAR* ApplicationName,
	WCHAR* CommandLine,
	WCHAR* CurrentDirectory,
	long long TotUserTimeLimit,		//整个沙盒的时间限制，msdn上说是100纳秒为单位。（奇怪？我觉得是10000*秒）设为-1不设置时间限制
	long long TotMemoryLimit,		//整个沙盒的内存限制，Byte为单位。设为-1不设置内存限制
	int CpuRateLimit,				//CPU限制，1-10000，（10000意味着可以用100%的CPU）不可以是0。设为-1不设置限制
	BOOL bLimitPrivileges,			//限制权限
	PBYTE pExternalData,			//附带的信息
	SANDBOX_CALLBACK CallbackFunc	//回调函数
	)
{
	BOOL bSuccess = FALSE;
	STARTUPINFOW SysInfo = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	HANDLE hJob = 0;
	HANDLE hPipeInRead = 0, hPipeInWrite = 0;
	HANDLE hPipeOutRead = 0, hPipeOutWrite = 0;

	pSANDBOX Sandbox = malloc(sizeof(SANDBOX));
	if (!Sandbox) return 0;

	ZeroMemory(Sandbox, sizeof(SANDBOX));
	__try
	{
		//创建ID
		Sandbox->SandboxID = InterlockedIncrement64(&AllocSandboxID);
		Sandbox->pExternalData = pExternalData;
		Sandbox->SandboxCallback = CallbackFunc;


		//创建作业
		hJob = CreateJobObject(0, 0);
		if (!hJob)__leave;

		{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendLimit = { 0 };
			if (TotUserTimeLimit != -1)
			{
				ExtendLimit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = TotUserTimeLimit;
				ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
			}
			if (TotMemoryLimit != -1)
			{
				ExtendLimit.JobMemoryLimit = (SIZE_T)TotMemoryLimit;
				ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
			}
			ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			bSuccess = SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &ExtendLimit, sizeof(ExtendLimit));
			if (!bSuccess)__leave;
			bSuccess = FALSE;


			JOBOBJECT_BASIC_UI_RESTRICTIONS UIRestriction = { 0 };
			UIRestriction.UIRestrictionsClass = JOB_OBJECT_UILIMIT_NONE;
			UIRestriction.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_EXITWINDOWS | //阻止进程注销，关机，重启或断开系统电源
				JOB_OBJECT_UILIMIT_READCLIPBOARD | //阻止进程读取剪贴板中的内容。
				JOB_OBJECT_UILIMIT_WRITECLIPBOARD | //阻止进程清除剪贴板中的内容。
				JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS | //阻止进程更改系统参数。
				JOB_OBJECT_UILIMIT_DISPLAYSETTINGS | //进程更改显示设置。
				JOB_OBJECT_UILIMIT_GLOBALATOMS | //为作业指定其专有的全局原子表，并限定作业中的进程只能访问此作业的表。
				JOB_OBJECT_UILIMIT_DESKTOP | //阻止进程创建或切换桌面。
				JOB_OBJECT_UILIMIT_HANDLES; //阻止作业中的进程使用同一个作业外部的进程所创建的用户对象( 如HWND) 。
			bSuccess = SetInformationJobObject(hJob, JobObjectBasicUIRestrictions, &UIRestriction, sizeof(UIRestriction));
			if (!bSuccess)__leave;
			bSuccess = FALSE;

			if (CpuRateLimit != -1)
			{
				JOBOBJECT_CPU_RATE_CONTROL_INFORMATION CpuControl = { 0 };
				CpuControl.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
				CpuControl.CpuRate = CpuRateLimit;
				bSuccess = SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &CpuControl, sizeof(CpuControl));
				if (!bSuccess)__leave;
				bSuccess = FALSE;
			}

			JOBOBJECT_ASSOCIATE_COMPLETION_PORT JobCompletionPort = { 0 };
			JobCompletionPort.CompletionPort = JobObjCompPort;
			JobCompletionPort.CompletionKey = Sandbox;
			SetInformationJobObject(hJob, JobObjectAssociateCompletionPortInformation, &JobCompletionPort, sizeof(JobCompletionPort));
			/*这里！！！加入Job的IOCP，CreateSimpleSandbox调用者应该提供一个回调函数用以通知事件发生。
				然后这个Sandbox加入到链表里以便枚举，清理
				写 FreeSimpleSandboxW。进程都结束后沙箱对象会仍然保留等手动释放
				Terminate整个job，释放对象，从链表中移除*/
		}


		SysInfo.cb = sizeof(STARTUPINFO);


		
		//创建管道
		{
			
			CreateOverlappedNamedPipePair(&hPipeInRead, &hPipeInWrite, PIPEIO_BUFSIZE);
			CreateOverlappedNamedPipePair(&hPipeOutRead, &hPipeOutWrite, PIPEIO_BUFSIZE);
			SysInfo.hStdInput = hPipeInRead;
			SysInfo.hStdOutput = SysInfo.hStdError = hPipeOutWrite;
			SysInfo.dwFlags |= STARTF_USESTDHANDLES;

			CreateIoCompletionPort(hPipeInWrite, PipeIOCompPort, (ULONG_PTR)Sandbox, 0);
			CreateIoCompletionPort(hPipeOutRead, PipeIOCompPort, (ULONG_PTR)Sandbox, 0);
		}

		
		SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		BOOL bCreated;
		if (bLimitPrivileges)
		{
			bCreated = CreateLimitedProcessW(ApplicationName, CommandLine, 0, 0, TRUE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, CurrentDirectory, &SysInfo, &ProcInfo);
		}
		else
		{
			bCreated = CreateProcessW(ApplicationName, CommandLine, 0, 0, TRUE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, CurrentDirectory, &SysInfo, &ProcInfo);
		}
		SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, 0);

		CloseHandle(hPipeInRead);
		CloseHandle(hPipeOutWrite);

		if (!bCreated)
		{
			__leave;
		}
		Sandbox->hPipeInWrite = hPipeInWrite;
		Sandbox->hPipeOutRead = hPipeOutRead;

		if (!AssignProcessToJobObject(hJob, ProcInfo.hProcess))
		{
			__leave;
		}

		if (!bCreated)
		{
			__leave;
		}
		Sandbox->hProcess = ProcInfo.hProcess;
		Sandbox->hJob = hJob;




		//加入链表
		AcquireSRWLockExclusive(&SandboxListLock);

		pSANDBOX_CHAIN OrgHead = RootSandbox;
		RootSandbox = Sandbox;
		Sandbox->NextSandbox = OrgHead;
		if (OrgHead)
		{
			OrgHead->LastSandbox = Sandbox;
		}

		ReleaseSRWLockExclusive(&SandboxListLock);

		//设置计时器
		if (TotUserTimeLimit != -1)
		{
			AcquireSRWLockExclusive(&PassArgTimerLock);
			g_ArgTimeLimit = TotUserTimeLimit; // In
			g_ArgSandboxID = Sandbox->SandboxID; // In
			SignalObjectAndWait(EventPassArgStart, EventPassArgEnd, INFINITE, FALSE);
			Sandbox->hWaitableTimer = hWaitableTimer;   // Out
			ReleaseSRWLockExclusive(&PassArgTimerLock);

			if (!Sandbox->hWaitableTimer)
			{
				TerminateJobObject(Sandbox->hJob, 1);
				Sandbox->ExitReason = SANDBOX_ER_ERROR;
				//这里leave了，那一头Job的IOCP就要遭殃。对象被删了。所以这里当成功处理
			}
		}
		

		//读取Pipe
		PipeIORead(hPipeOutRead);

		ResumeThread(ProcInfo.hThread);
		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			if (ProcInfo.hProcess)
			{
				TerminateProcess(ProcInfo.hProcess, 1);
				CloseHandle(ProcInfo.hProcess);
			}
			if (hJob)
			{
				CloseHandle(hJob);
			}

			if (Sandbox)
			{
				free(Sandbox);
				Sandbox = NULL;
			}

		}
		if (ProcInfo.hThread)
		{
			CloseHandle(ProcInfo.hThread);
		}
	}

	return Sandbox;
}

int FreeSimpleSandbox(pSANDBOX Sandbox)
{
	//移除
	AcquireSRWLockExclusive(&SandboxListLock);

	if (Sandbox->LastSandbox)
	{
		Sandbox->LastSandbox->NextSandbox = Sandbox->NextSandbox;
	}
	else
	{
		RootSandbox = Sandbox->NextSandbox;
	}

	if (Sandbox->NextSandbox)
	{
		Sandbox->NextSandbox->LastSandbox = Sandbox->LastSandbox;
	}
	ReleaseSRWLockExclusive(&SandboxListLock);

	TerminateJobObject(Sandbox->hJob, 1);
	if (Sandbox->hPipeInWrite) CloseHandle(Sandbox->hPipeInWrite);
	if (Sandbox->hPipeOutRead) CloseHandle(Sandbox->hPipeOutRead);
	CloseHandle(Sandbox->hProcess);
	CloseHandle(Sandbox->hJob);
	return 0;
}

unsigned __stdcall JobObjCompletionPort(LPVOID Args)
{
	while (1)
	{
		DWORD ByteTrans;
		LPOVERLAPPED lpOverlapped;
		pSANDBOX  pSandbox;
		GetQueuedCompletionStatus(JobObjCompPort, &ByteTrans, (PULONG_PTR)(&pSandbox), (LPOVERLAPPED*)&lpOverlapped, INFINITE);
		
		switch (ByteTrans)
		{
		case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
			//由于其他IO包可能晚于该IO包处理，而导致结果错误
			//甚至在极端情况下这个函数会清理掉Sandbox对象，等下一个包到来之时则会访问非法地址
			//TODO: 用平衡树，map替代这个链表。然后用SandboxID来校验对象是否存在
			//这是我不想要的。（当然，这种情况应该尽量考虑进去）
			//所以这里Sleep多次，将执行权交给其他线程
			for (int i = 0; i < 16; i++)Sleep(0);
			SANDBOX_CALLBACK Callback;
			PBYTE pData;
			AcquireSRWLockShared(&SandboxListLock);
				Callback = pSandbox->SandboxCallback;
				pData = pSandbox->pExternalData;
			ReleaseSRWLockShared(&SandboxListLock);
			if (Callback)
			{
				Callback(pSandbox, pData, SANDBOX_EVTNT_PROCESS_ZERO, 0, 0);
			}
			
			break;
		case JOB_OBJECT_MSG_EXIT_PROCESS:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_EXIT;
			}
			break;
		case JOB_OBJECT_MSG_END_OF_JOB_TIME:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_TIMEOUT;
			}
			break;
		case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_ABNORMAL;
			}
			break;
		case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
			//内存超出上限，记录下来
			pSandbox->bMemoryExceed = TRUE;
			break;
		case JOB_OBJECT_MSG_MY_CLEANUP:
			free(lpOverlapped);//释放申请的Overlapped结构
			return 0;
		}
	}

}


unsigned __stdcall PipeIOCompletionPort(LPVOID Args)
{
	while (1)
	{
		DWORD ByteTrans;
		pPIPEIO_PACK PipeIOPack;
		pSANDBOX  pSandbox;
		BOOL bResult = GetQueuedCompletionStatus(PipeIOCompPort, &ByteTrans, (PULONG_PTR)(&pSandbox), (LPOVERLAPPED*)&PipeIOPack, INFINITE);
		if (bResult)
		{
			//成功传输
			switch (PipeIOPack->PackMode)
			{
			case PACKMODE_READ:
			{
				SANDBOX_CALLBACK Callback;
				PBYTE pData;
				AcquireSRWLockShared(&SandboxListLock);
					Callback = pSandbox->SandboxCallback;
					pData = pSandbox->pExternalData;
				ReleaseSRWLockShared(&SandboxListLock);
				if (Callback)
				{
					Callback(pSandbox, pData, SANDBOX_EVENT_STD_OUTPUT, PipeIOPack->pData, ByteTrans);
				}

				free(PipeIOPack->pData);
				free(PipeIOPack);

				PipeIORead(pSandbox->hPipeOutRead);
			}
				break;
			case PACKMODE_WRITE:
				free(PipeIOPack);
				break;
			case PACKMODE_CLOSE://结束
				free(PipeIOPack);
				return 0;
			}
		}
		else
		{
			DWORD dwErr = GetLastError();
			switch (dwErr)
			{
			case ERROR_ABANDONED_WAIT_0:
				return 0;//完成端口关闭了，继续拆包啥也拆不出来
			default:
				//其他类型IO错误
				break;
			}
		}
	}

}


unsigned __stdcall TerminateJobTimerThread(LPVOID Args)
{
	int TimerCount = 0;
	while (1)
	{
		HANDLE WaitHandleList[2] = { SandboxCleaning ,EventPassArgStart };

		DWORD dwResult = WaitForMultipleObjectsEx(2, WaitHandleList, 0, INFINITE, TRUE);

		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			//Cleaning

			return 0; //退出线程
		case WAIT_OBJECT_0+1:
			//传参开始
		{
			HANDLE hTimer = CreateWaitableTimer(0, 0, 0);
			LARGE_INTEGER LargeInteger;
			LargeInteger.QuadPart = -g_ArgTimeLimit;
			if (hTimer)
			{
				SetWaitableTimer(hTimer, &LargeInteger, 0, TerminateJobTimerRoutine, (LPVOID)g_ArgSandboxID, FALSE);
			}
			hWaitableTimer = hTimer;
			SetEvent(EventPassArgEnd);
		}
			break;
		}
	}
	
	return 0;
}

void __stdcall TerminateJobTimerRoutine(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
	)
{
	AcquireSRWLockShared(&SandboxListLock);

	if (RootSandbox)
	{
		for (pSANDBOX_CHAIN pList = RootSandbox; pList; pList = pList->NextSandbox)
		{
			if (pList->SandboxID == (LONGLONG)lpArgToCompletionRoutine)
			{
				TerminateJobObject(pList->hJob, 1); //time over!
				if (pList->ExitReason == SANDBOX_ER_RUNNING)
				{
					pList->ExitReason = SANDBOX_ER_TIMEOUT;
				}
				break;
			}
		}
	}
	
	ReleaseSRWLockShared(&SandboxListLock);
	return;
}



BOOL CreateLimitedProcessW(
	 WCHAR lpApplicationName[],
	 WCHAR lpCommandLine[],
	 LPSECURITY_ATTRIBUTES lpProcessAttributes,
	 LPSECURITY_ATTRIBUTES lpThreadAttributes,
	 BOOL bInheritHandles,
	 DWORD dwCreationFlags,
	 LPVOID lpEnvironment,
	 WCHAR lpCurrentDirectory[],
	 LPSTARTUPINFOW lpStartupInfo,
	 LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL bSuccess = FALSE;
	BOOL bRet = FALSE;
	HANDLE hToken = 0;
	HANDLE hNewToken = 0;

	PTCHAR szIntegritySid = TEXT("S-1-16-4096"); // 低完整性 SID 
	PSID pIntegritySid = NULL;
	TOKEN_MANDATORY_LABEL TIL = { 0 };


	__try
	{
		bSuccess = OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED,
			&hToken);
		if (!bSuccess) __leave;

		bSuccess = DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityAnonymous, TokenPrimary, &hNewToken);//Anonymous应该是最低的了
		if (!bSuccess)__leave;

		bSuccess = ConvertStringSidToSid(szIntegritySid, &pIntegritySid);
		if (!bSuccess)__leave;

		TIL.Label.Attributes = SE_GROUP_INTEGRITY;
		TIL.Label.Sid = pIntegritySid;

		// 设置进程完整性级别 
		bSuccess = SetTokenInformation(hNewToken, TokenIntegrityLevel, &TIL,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid));
		if (!bSuccess)__leave;

		// 设置进程的 UI 权限级别 
		//bSuccess = SetTokenInformation(hNewToken, TokenIntegrityLevelDesktop,
		//	&TIL, sizeof(TOKEN_MANDATORY_LABEL) + RtlLengthSid(pIntegritySid));

		// 以低完整性创建新进程 
		bRet = CreateProcessAsUserW(hNewToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
		);
	}
	__finally
	{
		if (hToken)CloseHandle(hToken);
		if (hNewToken)CloseHandle(hNewToken);
	}

	return bRet;
}


BOOL CreateOverlappedNamedPipePair(PHANDLE hReadPipe, PHANDLE hWritePipe, DWORD nSize)
{
	WCHAR lpszPipename[128] = L"\\\\.\\pipe\\";
	for (int i = 9; i < 120; i++)
	{
		lpszPipename[i] = L'a' + rand() % 26;
	}

	(*hReadPipe) = CreateNamedPipeW(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_INBOUND |      // read/write access 
		FILE_FLAG_OVERLAPPED,     // overlapped mode 
		PIPE_TYPE_BYTE |       // byte-type pipe 
		PIPE_READMODE_BYTE |   // byte read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // unlimited instances 
		nSize,    // output buffer size 
		0,    // input buffer size 
		0,             // client time-out 
		NULL);
	if ((!(*hReadPipe)) || ((*hReadPipe) == INVALID_HANDLE_VALUE))
	{
		(*hReadPipe) = (*hWritePipe) = 0;
		return FALSE;
	}
	(*hWritePipe) = CreateFileW(lpszPipename, GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		FILE_FLAG_OVERLAPPED,              // default attributes 
		NULL);          // no template file 
	if ((!(*hWritePipe))||((*hWritePipe) == INVALID_HANDLE_VALUE))
	{
		CloseHandle((*hReadPipe));
		(*hReadPipe) = (*hWritePipe) = 0;
		return FALSE;
	}
	return TRUE;
}

BOOL PipeIORead(HANDLE PipeHandle)
{
	pPIPEIO_PACK PipeIOPack = malloc(sizeof(PIPEIO_PACK));
	BOOL bSuccess = FALSE;
	if (!PipeIOPack)
	{
		return FALSE;
	}
	__try
	{
		ZeroMemory(PipeIOPack, sizeof(PIPEIO_PACK));
		PipeIOPack->PackMode = PACKMODE_READ;
		PipeIOPack->pData = malloc(PIPEIO_BUFSIZE);
		if (!PipeIOPack->pData)
		{
			__leave;
		}
		ZeroMemory(PipeIOPack->pData, PIPEIO_BUFSIZE);
		DWORD BytesRead;
		BOOL bResult = ReadFile(PipeHandle, PipeIOPack->pData, PIPEIO_BUFSIZE, &BytesRead, PipeIOPack);
		if ((!bResult) && (GetLastError()!= ERROR_IO_PENDING))//失败了，失败码还不是ERROR_IO_PENDING
		{
			__leave;
		}
		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			if (PipeIOPack)
			{
				if (PipeIOPack->pData);
				{
					free(PipeIOPack->pData);
				}
				free(PipeIOPack);
			}
		}
	}
	return bSuccess;
}

BOOL PipeIOSendClose(HANDLE hCompPort)
{
	pPIPEIO_PACK PipeIOPack = malloc(sizeof(PIPEIO_PACK));
	if (!PipeIOPack)
	{
		return FALSE;
	}
	ZeroMemory(PipeIOPack, sizeof(PIPEIO_PACK));
	PipeIOPack->PackMode = PACKMODE_CLOSE;
	PostQueuedCompletionStatus(hCompPort, 0, 0, (LPOVERLAPPED)PipeIOPack);
	return TRUE;
}