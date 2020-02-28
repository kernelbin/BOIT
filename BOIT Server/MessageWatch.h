#include<Windows.h>
#include"SessionManage.h"

//监视消息类型
//MW 是 MessageWatch 的缩写
#define BOIT_MW_ALL 1 // 来自任何人，任何途径的消息
#define BOIT_MW_GROUP 2 // 来自指定群的所有消息
#define BOIT_MW_QQ 3 // 来自指定某个人，但可以是任何渠道的消息
#define BOIT_MW_GROUP_QQ 4 // 来自某个群里某个人的消息
#define BOIT_MW_QQ_PRIVATE 5 // 来自某个人的私聊消息



#define BOIT_MW_EVENT_TIMEOUT 1 // 等待超时
#define BOIT_MW_EVENT_MESSAGE 2



#define BOIT_MSGWATCH_PASS 0	// 放行消息
#define BOIT_MSGWATCH_BLOCK 1	// 截断消息

typedef INT(*MSGWATCH_CALLBACK)(long long MsgWatchID, PBYTE pData, UINT Event,
	long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);


typedef struct __tagMessageWatch BOIT_MSGWATCH, * pBOIT_MSGWATCH;

typedef struct __tagMessageWatch
{
	int WatchType;
	long long MsgWatchID;
	long long GroupID;
	long long QQID;
	BOOL ToBeFree;
	HANDLE hTimer;
	MSGWATCH_CALLBACK Callback;
	PBYTE pData;
	pBOIT_MSGWATCH Last;
	pBOIT_MSGWATCH Next;
}BOIT_MSGWATCH,* pBOIT_MSGWATCH;


CRITICAL_SECTION MsgWatchChainLock;

pBOIT_MSGWATCH RootMsgWatch;


int InitializeMessageWatch();

int FinalizeMessageWatch();

int RegisterMessageWatch(int WatchType,
	long long TimeOutInterval,
	pBOIT_SESSION boitSession,
	MSGWATCH_CALLBACK CallbackFunc,
	PBYTE pData);

int RemoveMessageWatch(pBOIT_MSGWATCH MsgWatch);

int RemoveMessageWatchByID(long long MsgWatchAllocID);

//int MarkFreeMessageWatchByID(long long MsgWatchAllocID);


int MessageWatchFilter(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);