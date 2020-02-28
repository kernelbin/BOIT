#pragma once
#include<Windows.h>
#include"CommandManager.h"

int CmdMsg_qwq_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR * Msg);
int CmdMsg_about_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_boast_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_help_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_run_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_savecode_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_runcode_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_stop_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_q_and_a_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_cat_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_meow_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);
int CmdMsg_version_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);



int CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);
