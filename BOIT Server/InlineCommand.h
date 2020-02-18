#pragma once
#include<Windows.h>
#include"CommandManager.h"

MSGPROC CmdMsg_qwq_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR * AnonymousName, WCHAR * Msg);
MSGPROC CmdMsg_about_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_commingfeature_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_help_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_run_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_stop_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);

EVENTPROC CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);
