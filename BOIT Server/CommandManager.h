#pragma once
#include<Windows.h>
#include"CommandEvent.h"
#include"SessionManage.h"

typedef UINT_PTR PARAMA;
typedef UINT_PTR PARAMB;

//指令链表数据结构定义

#define COMMAND_MAX_ALIAS 4 // 最多名称


typedef struct __tagCommand BOIT_COMMAND, * pBOIT_COMMAND;


typedef INT(*MSGPROC)(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);//回调函数定义
typedef INT(*EVENTPROC)(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);//回调函数定义


typedef struct __tagCommand
{
	unsigned int CommandID;

	WCHAR* CommandName[COMMAND_MAX_ALIAS];//指令名称
	int AliasCount;//名称，或者说是别名的数量
	MSGPROC MessageProc;//接收消息专用的回调函数
	EVENTPROC EventProc;//处理各类事件的回调函数
	WCHAR* ManualMsg;
	int MatchMode;

	pBOIT_COMMAND NextCommand;
}BOIT_COMMAND,*pBOIT_COMMAND;

pBOIT_COMMAND RootCommand;

SRWLOCK CommandChainLock;//访问指令链用的锁

int CommandAllocID;

#define BOIT_MATCH_FULL 1 //全字匹配，大小写不敏感
#define BOIT_MATCH_FULL_CASE 2 //全字匹配，大小写敏感
#define BOIT_MATCH_PARAM 3 //按第一个参数匹配，大小写不敏感
#define BOIT_MATCH_PARAM_CASE 4 //按第一个参数匹配，大小写敏感
#define BOIT_MATCH_HEAD 5 //按前N个字符匹配，大小写不敏感
#define BOIT_MATCH_HEAD_CASE 6 //按前N个字符匹配，大小写敏感




int InitializeCommandManager();

int FinalizeCommandManager();

pBOIT_COMMAND RegisterCommandEx(WCHAR* CommandName, MSGPROC MessageProc, EVENTPROC EventProc, WCHAR* ManualMsg, int MatchMode);

pBOIT_COMMAND RegisterCommand(WCHAR* CommandName, MSGPROC MessageProc, WCHAR* ManualMsg, int MatchMode);

int RemoveCommand(pBOIT_COMMAND Command);

int FreeCommand(pBOIT_COMMAND Command);

int BroadcastCommandEvent(UINT Event, PARAMA ParamA, PARAMB ParamB);

int SendCommandEvent(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);

int AddCommandAlias(pBOIT_COMMAND Command, WCHAR* AliasName);

BOOL CheckIsCommand(WCHAR* Msg, int* PrefixLen);

int GetCmdParamLen(WCHAR* String);

int GetCmdParamWithEscapeLen(WCHAR* String);

int CmdParamUnescape(WCHAR* String, WCHAR* UnescapeStr);

int GetCmdSpaceLen(WCHAR* String);

int CommandHandler(pBOIT_SESSION boitSession, WCHAR* Msg);



int RegisterInlineCommand();//注册所有自带指令