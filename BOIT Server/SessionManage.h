#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"

#define BOITSESS_TYPE_ERROR -1

#define BOITSESS_TYPE_NULL 0
#define BOITSESS_TYPE_GROUP 1
#define BOITSESS_TYPE_PRIVATE 2

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

int GetBOITSessionType(pBOIT_SESSION boitSession);

long long GetBOITSessionQQID(pBOIT_SESSION boitSession);

long long GetBOITSessionGroupID(pBOIT_SESSION boitSession);
