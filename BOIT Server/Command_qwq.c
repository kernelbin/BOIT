#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"

int CmdMsg_qwq_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR * Msg)
{
	
	SendBackMessage(boitSession, Corpus_Cute());
	return 0;
}