#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_boast_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"我可以查询codeforce上题目的题面和测试数据！",
		L"洛谷上的题目和题解我也可以找到√",
		L"LOJ, UOJ的题目我也可以查哦",
		L"OI Wiki上的文档也可以查看",
		L"数列找规律不如直接在bot上OEIS查询√",
		L"什么时候通过图灵测试？我早就通过图灵测试了你不知道吗？"
	};
	SendBackMessage(boitSession, ReplyMessage[rand() % _countof(ReplyMessage)]);

	switch (rand() % 20)
	{
	case 0:
		SendBackMessage(boitSession, L"骗人？");
		SendBackMessage(boitSession, L"吹牛逼不能算骗人.......吹牛逼......bot的事，能叫骗人吗？");
		break;
	case 1:
		SendBackMessage(boitSession, L"为什么我还不会做这些？那你得去问kernel.bin了");
		break;
	}
	return 0;
}