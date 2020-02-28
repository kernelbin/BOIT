#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_qwq_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR * Msg)
{
	WCHAR ReplyMessage[][16] = {
		L"pwp",
		L"qwq",
		L"/w\\",
		L"QwQ",
		L"/qwq\\",
		L"©d(¨R¨Œ¨Q*)o",
		L"`(*>©n<*)¡ä",
		L"(*¨R¨Œ¨Q)o¡î" ,
		L"(*/¦Ø£Ü*)"};
	SendBackMessage(boitSession, ReplyMessage[rand() % _countof(ReplyMessage)]);
	return 0;
}