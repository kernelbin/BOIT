#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

COMPROC Command_about_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	SendBackMessage(GroupID, QQID, L"BOIT 2.0 MadeBy kernel.bin");
	SendBackMessage(GroupID, QQID, L"OI + BOT = BOIT");
	SendBackMessage(GroupID, QQID, L"GithubÁ´½Ó -> https://github.com/kernelbin/BOIT");
	return 0;
}