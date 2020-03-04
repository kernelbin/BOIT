#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Global.h"
#include"DirManagement.h"

int CmdMsg_stop_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if (CheckUserToken(boitSession->QQID, L"PrivilegeBOITStop"))
	{
		SendBackMessage(boitSession, L"Goodbye~");
		SetEvent(hEventServerStop);
	}
	else
	{
		SendBackMessage(boitSession, Corpus_NoPrivilege());
	}
	
	return 0;
}