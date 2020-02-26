#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"SharedMemStruct.h"
#include<wchar.h>

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
				swprintf_s(CatMessage, _countof(CatMessage), L"àÛß´àÛß´£¬ %ls ÄãÒ²ÊÇÍÃ×Ó°¡", GroupMemberInfo.NickName);
			}
			else if (QQID == 1976658142)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇºÓÀê°¡", GroupMemberInfo.NickName);
			}
			else
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇÃ¨Ã¨°¡", GroupMemberInfo.NickName);
			}
			
		}
	}
	else
	{
		BOIT_STRANGER_INFO StrangerInfo;
		SendEventGetStrangerInfo(QQID, TRUE, &StrangerInfo);

		if (QQID == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"àÛß´àÛß´£¬ %ls ÄãÒ²ÊÇÍÃ×Ó°¡", StrangerInfo.NickName);
		}
		else if (QQID == 1976658142)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇºÓÀê°¡", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇÃ¨Ã¨°¡", StrangerInfo.NickName);
		}
		
	}
	SendBackMessage(GroupID, QQID, CatMessage);
	return 0;
}




int CmdMsg_meow_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
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
			RetrieveGroupMemberInfo(GroupID, QQID, TRUE, &GroupMemberInfo);
			WCHAR* ChosenName;
			if (GroupMemberInfo.CardName[0])
			{
				ChosenName = GroupMemberInfo.CardName;
			}
			else
			{
				ChosenName = GroupMemberInfo.NickName;
			}
			if (QQID == 693511570)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"àÛß´àÛß´£¬ %ls ÄãÒ²ÊÇÍÃ×Ó°¡", ChosenName);
			}
			else if (QQID == 1976658142)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇºÓÀê°¡", ChosenName);
			}
			else
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇÃ¨Ã¨°¡", ChosenName);
			}

		}
	}
	else
	{
		BOIT_STRANGER_INFO StrangerInfo;
		RetrieveStrangerInfo(QQID, TRUE, &StrangerInfo);

		if (QQID == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"àÛß´àÛß´£¬ %ls ÄãÒ²ÊÇÍÃ×Ó°¡", StrangerInfo.NickName);
		}
		else if (QQID == 1976658142)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇºÓÀê°¡", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"ß÷ß÷ß÷£¬ %ls ÄãÒ²ÊÇÃ¨Ã¨°¡", StrangerInfo.NickName);
		}

	}
	SendBackMessage(GroupID, QQID, CatMessage);
	return 0;
}