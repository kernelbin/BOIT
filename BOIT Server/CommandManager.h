#pragma once
#include<Windows.h>

typedef INT(*COMPROC)(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);//回调函数定义

//指令链表数据结构定义

#define COMMAND_MAX_ALIAS 4 // 最多名称


typedef struct __tagCommand BOIT_COMMAND, * pBOIT_COMMAND;

typedef struct __tagCommand
{
	unsigned int CommandID;

	WCHAR* CommandName[COMMAND_MAX_ALIAS];//指令名称
	int AliasCount;//名称，或者说是别名的数量
	COMPROC CommandProc;
	WCHAR* ManualMsg;
	int MatchMode;

	pBOIT_COMMAND NextCommand;
}BOIT_COMMAND,*pBOIT_COMMAND;

pBOIT_COMMAND RootCommand;

int CommandAllocID;

#define BOIT_MATCH_FULL 1 //全字匹配，大小写不敏感
#define BOIT_MATCH_FULL_CASE 2 //全字匹配，大小写敏感
#define BOIT_MATCH_PARAM 3 //按第一个参数匹配，大小写不敏感
#define BOIT_MATCH_PARAM_CASE 4 //按第一个参数匹配，大小写敏感
#define BOIT_MATCH_HEAD 5 //按前N个字符匹配，大小写不敏感
#define BOIT_MATCH_HEAD_CASE 6 //按前N个字符匹配，大小写敏感




int InitializeCommandManager();

int FinalizeCommandManager();

pBOIT_COMMAND RegisterCommand(WCHAR* CommandName, COMPROC CommandProc, WCHAR* ManualMsg, int MatchMode);

int RemoveCommand(pBOIT_COMMAND Command);

BOOL CheckIsCommand(WCHAR* Msg, int* PrefixLen);

int CommandHandler(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);



int RegisterInlineCommand();//注册所有自带指令