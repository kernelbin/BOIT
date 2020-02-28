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
		SendBackMessage(boitSession, L"Oops... 您没有适当的权限进行操作");
	}
	
	return 0;
}