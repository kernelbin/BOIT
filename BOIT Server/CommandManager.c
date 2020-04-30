#include<Windows.h>
#include"CommandManager.h"
#include<wchar.h>
#include<strsafe.h>
#include"DirManagement.h"


int InitializeCommandManager()
{
	InitializeSRWLock(&CommandChainLock);
	CommandAllocID = 0;
	RootCommand = 0;
	return 0;
}

int FinalizeCommandManager()
{
	AcquireSRWLockExclusive(&CommandChainLock);
	if (!RootCommand)
	{
		return 0;
	}
	for (pBOIT_COMMAND pList = RootCommand; pList;)
	{
		pBOIT_COMMAND NextCommand = pList->NextCommand;
		FreeCommand(pList);
		pList = NextCommand;
	}
	ReleaseSRWLockExclusive(&CommandChainLock);
	RootCommand = 0;
	return 0;
}


pBOIT_COMMAND RegisterCommandEx(WCHAR* CommandName, MSGPROC MessageProc, EVENTPROC EventProc, WCHAR* ManualMsg, int MatchMode)
{
	BOOL bSuccess = FALSE;
	pBOIT_COMMAND Command = malloc(sizeof(BOIT_COMMAND));
	if (!Command)return 0;
	ZeroMemory(Command, sizeof(BOIT_COMMAND));
	__try
	{
		int StrLength = lstrlenW(CommandName);
		Command->CommandName[0] = malloc(sizeof(WCHAR) * (StrLength + 1));
		if (!Command->CommandName[0])__leave;
		StringCchCopyW(Command->CommandName[0], StrLength + 1, CommandName);
		Command->CommandName[0][StrLength] = 0;

		Command->AliasCount++;

		Command->MessageProc = MessageProc;
		Command->EventProc = EventProc;

		StrLength = lstrlenW(ManualMsg);
		Command->ManualMsg = malloc(sizeof(WCHAR) * (StrLength + 1));
		if (!Command->ManualMsg)__leave;
		StringCchCopyW(Command->ManualMsg, StrLength + 1, ManualMsg);
		Command->ManualMsg[StrLength] = 0;

		Command->MatchMode = MatchMode;

		Command->CommandID = ++CommandAllocID;
		//插入链表
		AcquireSRWLockExclusive(&CommandChainLock);

		if (RootCommand)
		{
			pBOIT_COMMAND pList;
			for (pList = RootCommand; pList->NextCommand; pList = pList->NextCommand);
			pList->NextCommand = Command;
		}
		else
		{
			RootCommand = Command;
		}

		ReleaseSRWLockExclusive(&CommandChainLock);

		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			if (Command->CommandName[0])free(Command->CommandName[0]);
			if (Command->ManualMsg)free(Command->ManualMsg);
			free(Command);
			Command = 0;
		}
		
	}


	
	return Command;
}



pBOIT_COMMAND RegisterCommand(WCHAR* CommandName, MSGPROC MessageProc, WCHAR* ManualMsg, int MatchMode)
{
	return RegisterCommandEx(CommandName, MessageProc, NULL, ManualMsg, MatchMode);
}

int RemoveCommand(pBOIT_COMMAND Command)
{
	//根据CommandID找到链表里对应的项，从列表里移除出来然后删了
	int CommandID = Command->CommandID;
	pBOIT_COMMAND pTargetCommand = 0;
	if (!RootCommand)
	{
		return 0;//滚
	}
	for (pBOIT_COMMAND pList = RootCommand; pList->NextCommand; pList = pList->NextCommand)
	{
		if (pList->NextCommand->CommandID == CommandID)
		{
			//找到了，删除
			pTargetCommand = pList->NextCommand;
			pList->NextCommand = pList->NextCommand->NextCommand;
			break;
		}
	}

	if (pTargetCommand)
	{
		FreeCommand(pTargetCommand);
	}
	return 0;
}

int FreeCommand(pBOIT_COMMAND Command)
{
	for (int i = 0; i < Command->AliasCount; i++)
	{
		free(Command->CommandName[i]);
	}

	free(Command->ManualMsg);

	free(Command);
	return 0;
}



int BroadcastCommandEvent(UINT Event, PARAMA ParamA, PARAMB ParamB)//对指令链里所有指令广播事件
{
	AcquireSRWLockShared(&CommandChainLock);
	for (pBOIT_COMMAND pList = RootCommand; pList; pList = pList->NextCommand)
	{
		SendCommandEvent(pList, Event, ParamA, ParamB);
	}
	ReleaseSRWLockShared(&CommandChainLock);
	return 0;
}

int SendCommandEvent(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	if (pCmd && pCmd->EventProc)
	{
		return pCmd->EventProc(pCmd, Event, ParamA, ParamB);
	}
	return 0;
}

int AddCommandAlias(pBOIT_COMMAND Command,WCHAR * AliasName)
{
	AcquireSRWLockExclusive(&CommandChainLock);
	if (Command->AliasCount == COMMAND_MAX_ALIAS)
	{
		return 0;
	}
	int AliasLen = lstrlenW(AliasName);
	Command->CommandName[Command->AliasCount] = malloc(sizeof(WCHAR) * (AliasLen + 1));
	if (!Command->CommandName[Command->AliasCount])return 0;
	StringCchCopyW(Command->CommandName[Command->AliasCount], AliasLen + 1, AliasName);
	Command->CommandName[Command->AliasCount++][AliasLen] = 0;
	
	ReleaseSRWLockExclusive(&CommandChainLock);
	return 0;
}




BOOL CheckIsCommand(WCHAR* Msg, int* PrefixLen)
{
	//TODO: 从配置中读取指令前缀和昵称
	if (Msg[0] == L'#')
	{
		if (PrefixLen)
		{
			*PrefixLen = 1;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


int GetCmdParamLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] == L' ' ||
			String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}
	}
	return i;
}

int GetCmdParamWithEscapeLen(WCHAR* String)
{
	int i;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				i++;
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
		}
	}
	return i;
}


int CmdParamUnescape(WCHAR* String,WCHAR * UnescapeStr)
{
	int i = 0;
	int j = 0;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
	}
	UnescapeStr[j++] = 0;
	return i;
}




int GetCmdSpaceLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\r' &&
			String[i] != L'\n' &&
			String[i] != L'\t')
		{
			break;
		}
	}
	return i;
}



int CommandHandler(pBOIT_SESSION boitSession, WCHAR* Msg)
{
	CheckPerUserDataExist(GetBOITSessionQQID(boitSession));
	CheckPerGroupDataExist(GetBOITSessionGroupID(boitSession));
	AcquireSRWLockShared(&CommandChainLock);
	if (!RootCommand)
	{
		return 0;
	}
	for (pBOIT_COMMAND pList = RootCommand; pList; pList = pList->NextCommand)
	{
		for (int i = 0; i < pList->AliasCount; i++)
		{
			BOOL bMatch = FALSE;

			switch (pList->MatchMode)
			{
			case BOIT_MATCH_FULL:
				if (_wcsicmp(pList->CommandName[i], Msg) == 0) bMatch = TRUE;
				break;
			case BOIT_MATCH_FULL_CASE:
				if (wcscmp(pList->CommandName[i], Msg) == 0) bMatch = TRUE;
				break;
			case BOIT_MATCH_PARAM:
			{
				int ParamLen = GetCmdParamLen(Msg);
				if (ParamLen == wcslen(pList->CommandName[i]) && _wcsnicmp(pList->CommandName[i], Msg, ParamLen) == 0) bMatch = TRUE;
			}
				break;
			case BOIT_MATCH_PARAM_CASE:
			{
				int ParamLen = GetCmdParamLen(Msg);
				if (ParamLen == wcslen(pList->CommandName[i]) && wcsncmp(pList->CommandName[i], Msg, ParamLen) == 0) bMatch = TRUE;
			}
				break;
			case BOIT_MATCH_HEAD:
			{
				int CommandLen = wcslen(pList->CommandName[i]);
				if (_wcsnicmp(pList->CommandName[i], Msg, CommandLen) == 0) bMatch = TRUE;
			}
				break;
			case BOIT_MATCH_HEAD_CASE:
			{
				int CommandLen = wcslen(pList->CommandName[i]);
				if (wcsncmp(pList->CommandName[i], Msg, CommandLen) == 0) bMatch = TRUE;
			}
				break;
			}

			if (bMatch)
			{
				pList->MessageProc(pList, boitSession, Msg);
			}
		}
	}
	ReleaseSRWLockShared(&CommandChainLock);
	return 0;
}


