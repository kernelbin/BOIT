#include<Windows.h>
#include"APITransfer.h"
#include"CommandManager.h"
#include"InlineCommand.h"

int RegisterInlineCommand()
{
	pBOIT_COMMAND Command_qwq = RegisterCommand(L"qwq", Command_qwq_Proc, L"卖萌", BOIT_MATCH_FULL);
	AddCommandAlias(Command_qwq, L"pwp");
	RegisterCommand(L"about", Command_about_Proc, L"关于", BOIT_MATCH_FULL);
	RegisterCommand(L"TreeNewBee", Command_commingfeature_Proc, L"吹牛逼(开发中的功能)", BOIT_MATCH_FULL);
	RegisterCommand(L"help", Command_help_Proc, L"帮助信息", BOIT_MATCH_PARAM);
	
	return 0;
}