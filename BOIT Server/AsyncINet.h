#pragma once

#include<Windows.h>
#include<WinInet.h>
#include"VBuffer.h"



#define ASYNCINET_REASON_SUCCESS 1
#define ASYNCINET_REASON_FAILED 2


typedef
int
(*ASYNC_INET_CALLBACK)(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
	);


#define ASYNC_INET_BUF 4096

typedef struct __tagAsyncINet
{
	HINTERNET hInet;
	HINTERNET hConnect;
}ASYNCINET_INFO, * pASYNCINET_INFO;

typedef struct __tagAsyncRequest
{
	BYTE ReadBuffer[ASYNC_INET_BUF];
	DWORD BytesRead;

	HINTERNET hRequest;
	pVBUF vBuffer;
	BOOL bRequestComplete;

	PBYTE ExtData;

	ASYNC_INET_CALLBACK AsyncCallback;
}ASYNC_REQUEST, * pASYNC_REQUEST;



pASYNCINET_INFO AsyncINetInit(WCHAR* ServerURL);

BOOL AsyncINetCleanup(pASYNCINET_INFO AsyncINetInfo);

BOOL AsyncRequestGet(pASYNCINET_INFO AsyncINetInfo, WCHAR* Url, PBYTE ExtData, ASYNC_INET_CALLBACK AsyncCallback);

