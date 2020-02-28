#include<Windows.h>
#include"SessionManage.h"

pBOIT_SESSION InitBOITSession(long long GroupID, long long QQID, WCHAR* AnonymousName, int SubType)
{
	pBOIT_SESSION boitSession = 0;
	boitSession = malloc(sizeof(BOIT_SESSION));
	if (!boitSession)
	{
		return 0;
	}
	boitSession->GroupID = GroupID;
	boitSession->QQID = QQID;
	boitSession->SubType = SubType;
	if (AnonymousName && AnonymousName[0])
	{
		wcscpy_s(boitSession->AnonymousName, BOIT_MAX_NICKLEN, AnonymousName);
	}
	else
	{
		boitSession->AnonymousName[0] = 0;
	}

	return boitSession;
}

pBOIT_SESSION DuplicateBOITSession(pBOIT_SESSION SourceSession)
{
	pBOIT_SESSION boitSession = 0;
	boitSession = malloc(sizeof(BOIT_SESSION));
	if (!boitSession)
	{
		return 0;
	}
	boitSession->GroupID = SourceSession->GroupID;
	boitSession->QQID = SourceSession->QQID;
	boitSession->SubType = SourceSession->SubType;
	if (SourceSession->AnonymousName[0])
	{
		wcscpy_s(boitSession->AnonymousName, BOIT_MAX_NICKLEN, SourceSession->AnonymousName);
	}

	return boitSession;
}


BOOL FreeBOITSession(pBOIT_SESSION boitSession)
{
	if (boitSession)
	{
		free(boitSession);
	}
	return TRUE;
}