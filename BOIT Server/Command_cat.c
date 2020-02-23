#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"SharedMemStruct.h"
#include<wchar.h>

#include"SendEventDispatch.h" //这是临时的，后面封包进APITransfer
int CmdMsg_cat_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR CatMessage[128];
	
	if (GroupID)
	{
		if (QQID == 80000000)
		{
			return 0;
		}
		else
		{
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;
			SendEventGetGroupMemberInfo(GroupID, QQID, TRUE, &GroupMemberInfo);
			if (QQID == 693511570)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"噗呼噗呼， %ls 你也是兔子啊", GroupMemberInfo.NickName);
			}
			else
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"喵喵喵， %ls 你也是猫猫啊", GroupMemberInfo.NickName);
			}
			
		}
	}
	else
	{
		BOIT_STRANGER_INFO StrangerInfo;
		SendEventGetStrangerInfo(QQID, TRUE, &StrangerInfo);

		if (QQID == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"噗呼噗呼， %ls 你也是兔子啊", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"喵喵喵， %ls 你也是猫猫啊", StrangerInfo.NickName);
		}
		
	}
	SendBackMessage(GroupID, QQID, CatMessage);
	return 0;
}