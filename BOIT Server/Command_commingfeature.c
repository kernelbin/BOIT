#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

MSGPROC CmdMsg_commingfeature_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"我可以查询codeforce上的比赛时间呢，我还能查询tourist的profile！",
		L"我可以执行代码呢，什么语言都能运行！还支持数据输入和写入文件！",
		L"查询天气我也会的呢！一言查询也不是问题哦",
		L"洛谷上的题目我也可以找到√",
		L"播放歌曲对我也是小菜一碟哦",
		L"成语接龙和飞花令也是我的拿手长项",
		L"什么时候通过图灵测试？我早就通过图灵测试了你不知道吗？"
	};
	SendBackMessage(GroupID, QQID, ReplyMessage + rand() % _countof(ReplyMessage));

	switch (rand() % 20)
	{
	case 0:
		SendBackMessage(GroupID, QQID, L"骗人？");
		SendBackMessage(GroupID, QQID, L"吹牛逼不能算骗人.......吹牛逼......bot的事，能叫骗人吗？");
		break;
	case 1:
		SendBackMessage(GroupID, QQID, L"为什么我还不会做这些？那你得去问kernel.bin了");
		break;
	}
	return 0;
}