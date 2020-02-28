#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_about_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	SendBackMessage(boitSession, L"BOIT 2.0 MadeBy kernel.bin");
	SendBackMessage(boitSession, L"OI + BOT = BOIT");
	SendBackMessage(boitSession, L"GithubÁ´½Ó -> https://github.com/kernelbin/BOIT");
	return 0;
}