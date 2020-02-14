#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"DirManagement.h"

MSGPROC CmdMsg_run_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	//检查用户目录下是否有相应文件夹
	if (PerUserCreateDirIfNExist(QQID, L"Sandbox"))
	{
		//创建相应文件
	}
	if (PerUserCreateDirIfNExist(QQID, L"Compile"))
	{
		//创建相应文件
	}

	SendBackMessage(GroupID, QQID, L"还不能运行代码哦");
	return 0;
}

EVENTPROC CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_DIRINIT:
		//这里创建编译配置文件
		break;
	}
	return 0;
}