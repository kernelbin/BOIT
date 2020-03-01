#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_about_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	SendBackMessage(boitSession, L"BOIT 2.0 MadeBy kernel.bin");
	SendBackMessage(boitSession, L"OI + BOT = BOIT");
	SendBackMessage(boitSession, L"[CQ:share,url=https://github.com/kernelbin/BOIT,title=BOIT ÏîÄ¿ Github Á´½Ó,content=An open sourced bot for OIers,image=https://i.loli.net/2020/03/01/S2L8DRgje9zmxJ1.png]");

	return 0;
}