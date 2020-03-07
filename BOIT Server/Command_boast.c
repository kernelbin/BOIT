#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_boast_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"我可以查询codeforce上的比赛时间呢，还可以查询题目的题面和测试数据！",
		L"查询天气我也会的呢！一言查询也不是问题哦",
		L"洛谷上的题目我也可以找到√",
		L"播放歌曲对我也是小菜一碟哦",
		L"成语接龙和飞花令也是我的拿手长项",
		L"在bot上玩人力资源机也可以做到哦",
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