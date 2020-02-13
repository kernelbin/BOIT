#include<Windows.h>
#include"APITransfer.h"
#include"CommandManager.h"
#include"InlineCommand.h"

int RegisterInlineCommand()
{
	RegisterCommand(L"qwq", Command_qwq_Proc, L"ÂôÃÈ", BOIT_MATCH_HEAD);
	return 0;
}