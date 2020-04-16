#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_about_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	SendBackMessage(boitSession, L"BOIT 2.0 MadeBy kernel.bin with ❤");
	SendBackMessage(boitSession, L"OI + BOT = BOIT");
	SendBackMessage(boitSession, L"[CQ:share,url=https://github.com/kernelbin/BOIT,title=BOIT 项目 Github 链接,content=An open sourced bot for OIers,image=image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");

	return 0;
}