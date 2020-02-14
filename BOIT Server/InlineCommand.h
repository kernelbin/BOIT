#pragma once
#include<Windows.h>
#include"CommandManager.h"

MSGPROC CmdMsg_qwq_Proc(long long GroupID, long long QQID, WCHAR * AnonymousName, WCHAR * Msg);
MSGPROC CmdMsg_about_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_commingfeature_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_help_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);
MSGPROC CmdMsg_run_Proc(long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg);