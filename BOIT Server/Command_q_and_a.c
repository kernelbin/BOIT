#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_q_and_a_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"Q:我可以对bot做下流之事吗？\n\nA:除了 quine 和让 bot 发表不当言论以外的方式尝试对 bot 进行 CTF 或者攻击是允许的。\n（bot 被封号了没得玩的是你们哦）\n如果有发现安全漏洞，非常欢迎与作者交流。",
		L"Q:我可以把bot拉到其他群聊吗？\n\nA:在得到群主同意的情况下可以，请联系kernel.bin. 不过不保证 bot 全天 24h 运行。",
		L"Q:bot什么时候出新功能？\n\nA:取决于 kernel.bin 有多鸽子",
		L"Q:为什么突然减少了好多功能？\n\nA:因为我对整个 bot 进行了重写以提升运行速度和安全性",
		L"Q:为什么不能运行 xxx 语言？\n\nA:如果想要新加语言，请联系 kernel.bin",
		L"Q:bot开源吗？\n\nA:开源的，输入#about查看详情，欢迎 star (*≧▽≦)o☆",
		L"Q:运行代码是什么原理？\n\nA:在本地编译，并且通过管道重定向实现获取输出数据，并且通过一些手段限制进程权限" };

	SendBackMessage(GroupID, QQID, ReplyMessage[rand() % _countof(ReplyMessage)]);
	return 0;
}