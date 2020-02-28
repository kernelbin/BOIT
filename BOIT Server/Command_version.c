#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"BOITVersion.h"

#define __LPREFIX(quote) L##quote  
#define LPREFIX(quote) __LPREFIX(quote)

#define __NUM2LTEXT(quote) __LPREFIX(#quote)
#define NUM2LTEXT(quote) __NUM2LTEXT(quote)

int CmdMsg_version_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	SendBackMessage(boitSession,  L"BOIT 版本" NUM2LTEXT(VERSION_MAJOR) L"." NUM2LTEXT(VERSION_MINOR) L"  "
#ifdef VERSION_ALPHA
		L"Alpha"
#elif defined(VERSION_BETA)
		L"Beta"
#elif defined(VERSION_RC)
		L"RC"
#elif defined(VERSION_RELEASE)
		L"Release"
#endif
	);
	SendBackMessage(boitSession, L"构建时间：" LPREFIX(__DATE__) L" " LPREFIX(__TIME__));
	return 0;
}