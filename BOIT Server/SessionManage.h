#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


typedef struct __tagSession
{
	//sessionµÄ¼òµ¥Ìæ´ú
	long long QQID;
	long long GroupID;
	WCHAR AnonymousName[BOIT_MAX_NICKLEN];
}BOIT_SESSION, * pBOIT_SESSION;