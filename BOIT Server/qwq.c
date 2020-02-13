#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"

COMPROC Command_qwq_Proc(long long GroupID, long long QQID, WCHAR * AnonymousName, WCHAR * Msg)
{
	SendBackMessage(GroupID, QQID, L"pwp");
	return 0;
}