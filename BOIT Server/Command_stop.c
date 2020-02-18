#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Global.h"

MSGPROC CmdMsg_stop_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	SendBackMessage(GroupID, QQID, L"Goodbye~");
	SetEvent(hEventServerStop);
	return 0;
}