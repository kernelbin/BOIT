#pragma once
#include<Windows.h>

typedef struct __tagSandbox SANDBOX, * pSANDBOX, SANDBOX_CHAIN, * pSANDBOX_CHAIN;

//沙箱回调函数定义
typedef INT(*SANDBOX_CALLBACK)(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen);

//Callback事件
#define SANDBOX_EVTNT_PROCESS_ZERO 1
#define SANDBOX_EVENT_STD_OUTPUT 2


typedef struct __tagSandbox
{
	HANDLE hJob;
	HANDLE hProcess;

	LONGLONG SandboxID;
	HANDLE hWaitableTimer;

	UINT ExitReason;

	SANDBOX_CALLBACK SandboxCallback;
	PBYTE pExternalData;

	HANDLE hPipeInWrite;
	HANDLE hPipeOutRead;

	pSANDBOX_CHAIN LastSandbox;
	pSANDBOX_CHAIN NextSandbox;
}SANDBOX, * pSANDBOX, SANDBOX_CHAIN, * pSANDBOX_CHAIN;

pSANDBOX_CHAIN RootSandbox;


int InitializeSandbox(int JobObjCompThreadNum, int PipeIOCompThreadNum);

int FinalizeSandbox();

pSANDBOX CreateSimpleSandboxW(WCHAR* ApplicationName,
	WCHAR* CommandLine,
	WCHAR* CurrentDirectory,
	long long TotUserTimeLimit,		//整个沙盒的时间限制，msdn上说是100纳秒为单位。（奇怪？我觉得是10000*秒）设为-1不设置时间限制
	long long TotMemoryLimit,		//整个沙盒的内存限制，Byte为单位。设为-1不设置内存限制
	int CpuRateLimit,				//CPU限制，1-10000，（10000意味着可以用100%的CPU）不可以是0。设为-1不设置限制
	BOOL bLimitPrivileges,			//限制权限
	PBYTE pExternalData,			//附带的信息
	SANDBOX_CALLBACK CallbackFunc	//回调函数
);

int FreeSimpleSandbox(pSANDBOX Sandbox);

//用于Pipe的OverlappedIO包

#define PACKMODE_READ 1
#define PACKMODE_WRITE 2
#define PACKMODE_CLOSE 3

typedef struct __tagPipeIOPack
{
	OVERLAPPED Overlapped;
	int PackMode;
	PBYTE pData;
}PIPEIO_PACK,*pPIPEIO_PACK;

#define PIPEIO_BUFSIZE 1024



//退出原因定义 ExitReason -> ER
#define SANDBOX_ER_RUNNING 0	//还没退出
#define SANDBOX_ER_EXIT 1		//正常退出
#define SANDBOX_ER_TIMEOUT 2	//超出时间限制
#define SANDBOX_ER_ABNORMAL 3	//异常退出
#define SANDBOX_ER_KILLED 4		//被沙箱强制关闭了

#define SANDBOX_ER_ERROR 5


