#pragma once
#include<Windows.h>

#define BOIT_MAX_TEXTLEN 2048 //超出长度的一律截断

typedef struct __tagEventRecvStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;
			long long QQID;
		}GroupMsg;
		 
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;
		}PrivateMsg;
	}u;
}EVENT_RECV, *pEVENT_RECV;


typedef struct __tagEventSendStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;
		}GroupMsg;

		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;
		}PrivateMsg;
	}u;
}EVENT_SEND,*pEVENT_SEND;


typedef struct __tagSharedProcessInfo
{
	DWORD pid[2];
}SHARED_PROCESS_INFO, *pSHARED_PROCESS_INFO;
