#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


typedef struct __tagSession
{
	//sessionµÄ¼òµ¥Ìæ´ú
	long long QQID;
	long long GroupID;
	int SubType;
	WCHAR AnonymousName[BOIT_MAX_NICKLEN];
}BOIT_SESSION, * pBOIT_SESSION;

pBOIT_SESSION InitBOITSession(long long GroupID, long long QQID, WCHAR* AnonymousName, int SubType);

pBOIT_SESSION DuplicateBOITSession(pBOIT_SESSION SourceSession);

BOOL FreeBOITSession(pBOIT_SESSION boitSession);
