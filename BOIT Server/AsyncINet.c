#pragma once
#include<Windows.h>
#include<WinInet.h>
#include"VBuffer.h"
#include"AsyncINet.h"
#pragma comment(lib,"WinINet.lib")

VOID CALLBACK AsyncINetCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
);

pASYNCINET_INFO AsyncINetInit(WCHAR * ServerURL)
{
	pASYNCINET_INFO AsyncINetInfo = malloc(sizeof(ASYNCINET_INFO));
	ZeroMemory(AsyncINetInfo, sizeof(ASYNCINET_INFO));

	AsyncINetInfo->hInet = InternetOpenW(L"BOIT", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
	INTERNET_STATUS_CALLBACK pOldStatusCallback = InternetSetStatusCallbackW(AsyncINetInfo->hInet, AsyncINetCallback);

	AsyncINetInfo->hConnect = InternetConnectW(AsyncINetInfo->hInet,
		ServerURL, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	return AsyncINetInfo;
}

BOOL AsyncINetCleanup(pASYNCINET_INFO AsyncINetInfo)
{
	InternetCloseHandle(AsyncINetInfo->hConnect);
	InternetCloseHandle(AsyncINetInfo->hInet);
	free(AsyncINetInfo);
	return TRUE;
}

pASYNC_REQUEST AllocAsyncRequestStruct()
{
	pASYNC_REQUEST AsyncRequest = malloc(sizeof(ASYNC_REQUEST));
	ZeroMemory(AsyncRequest, sizeof(ASYNC_REQUEST));

	AsyncRequest->vBuffer = AllocVBuf();
	return AsyncRequest;
}

BOOL FreeAsyncRequestStruct(pASYNC_REQUEST AsyncRequest)
{
	FreeVBuf(AsyncRequest->vBuffer);
	free(AsyncRequest);
	return TRUE;
}

BOOL AsyncRequestGet(pASYNCINET_INFO AsyncINetInfo, WCHAR * Url, PBYTE ExtData, ASYNC_INET_CALLBACK AsyncCallback)
{
	WCHAR* rgpszAcceptTypes[] = { L"*/*", NULL };
	pASYNC_REQUEST AsyncRequest = AllocAsyncRequestStruct();
	AsyncRequest->AsyncCallback = AsyncCallback;
	AsyncRequest->ExtData = ExtData;
	AsyncRequest->hRequest = HttpOpenRequestW(AsyncINetInfo->hConnect, L"GET", Url,
		NULL, NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, AsyncRequest);

	
	BOOL x = HttpSendRequestW(AsyncRequest->hRequest, 0, 0, 0, 0);
}

VOID CALLBACK AsyncINetCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
)
{
	pASYNC_REQUEST AsyncINetReq = dwContext;
	BOOL bSuccess = FALSE;
	HINTERNET* a = lpvStatusInformation;
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_REQUEST_COMPLETE:
	{
		INTERNET_ASYNC_RESULT* AsyncResult = lpvStatusInformation;
		if (AsyncResult->dwResult == 0)
		{
			//Failed

		}
		else
		{
			if (!AsyncINetReq->bRequestComplete)
			{
				AsyncINetReq->bRequestComplete = TRUE;
			}
			else
			{
				//上次结果复制进缓冲区
				//if (QueryStruct->InetBuf.dwBufferLength < 1000)
			ReadNow:
				{
					int OrgLen = AsyncINetReq->vBuffer->Length;
					AddSizeVBuf(AsyncINetReq->vBuffer, AsyncINetReq->BytesRead);
					memcpy(AsyncINetReq->vBuffer->Data + OrgLen, AsyncINetReq->ReadBuffer, AsyncINetReq->BytesRead);
				}
			}


			ZeroMemory(AsyncINetReq->ReadBuffer, ASYNC_INET_BUF);

			BOOL bRet = InternetReadFile(AsyncINetReq->hRequest, AsyncINetReq->ReadBuffer, ASYNC_INET_BUF, &(AsyncINetReq->BytesRead));
			DWORD dwErr = GetLastError();
			if (!bRet)
			{
				if (dwErr == ERROR_IO_PENDING)
				{
					//如果是ERROR_IO_PENDING的话，继续下去等待执行完毕即可
					return;
				}
			}
			else
			{
				if (AsyncINetReq->BytesRead)
				{
					goto ReadNow;
				}
				AsyncINetReq->AsyncCallback(ASYNCINET_REASON_SUCCESS,
					AsyncINetReq->vBuffer, AsyncINetReq->ExtData);
				bSuccess = TRUE;
			}
		}

		//控制流流到这里的不是失败了就是结束了。清理。
		if (!bSuccess)
		{
			//失败通知
			AsyncINetReq->AsyncCallback(ASYNCINET_REASON_FAILED, 0, AsyncINetReq->ExtData);
		}
		InternetCloseHandle(AsyncINetReq->hRequest);
		FreeAsyncRequestStruct(AsyncINetReq);

	}
	}
	return;
}


