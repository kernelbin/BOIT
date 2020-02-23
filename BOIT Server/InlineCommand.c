#include<Windows.h>
#include"APITransfer.h"
#include"CommandManager.h"
#include"InlineCommand.h"


int RegisterInlineCommand()
{
	pBOIT_COMMAND Command_qwq = RegisterCommand(L"qwq", CmdMsg_qwq_Proc, L"卖萌", BOIT_MATCH_FULL);
	RegisterCommand(L"猫猫", CmdMsg_cat_Proc, L"猫猫", BOIT_MATCH_FULL);
	AddCommandAlias(Command_qwq, L"pwp");
	RegisterCommand(L"about", CmdMsg_about_Proc, L"关于", BOIT_MATCH_FULL);
	RegisterCommand(L"boast", CmdMsg_commingfeature_Proc, L"吹牛逼(开发中的功能)", BOIT_MATCH_FULL);
	RegisterCommandEx(L"run", CmdMsg_run_Proc, CmdEvent_run_Proc, L"运行代码", BOIT_MATCH_PARAM);
	RegisterCommand(L"help", CmdMsg_help_Proc, L"帮助信息", BOIT_MATCH_PARAM);
	RegisterCommand(L"q&amp;a", CmdMsg_q_and_a_Proc, L"常见问答", BOIT_MATCH_FULL);
	RegisterCommand(L"stop", CmdMsg_stop_Proc, L"关闭BOIT", BOIT_MATCH_FULL);


	return 0;
}