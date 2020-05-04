#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"


typedef struct __tagQwQMsgAndChangce
{
	WCHAR* Msg;
	int Chance;
}QWQ_MSG_AND_CHANCE;

QWQ_MSG_AND_CHANCE QwQMessage[] =
{
		L"pwp", 5,
		L"qwq", 5,
		L"owo", 5,
		L"/w\\", 5,
		L"QwQ", 5,
		L"/qwq\\", 3,
		L"ヾ(≧▽≦*)o", 3,
		L"`(*>﹏<*)′", 3,
		L"(*≧▽≦)o☆" , 3,
		L"(*/ω＼*)", 3,
		L"(。・∀・)ノ", 2,
		L"_(:з」∠)_", 2,
		L"(*´▽`*)❀", 2,
		L"(๑• . •๑)◞♡", 2,
		L"(´• ᵕ •`)*✲ﾟ*", 2,
		L"(´,,•∀•,,`)", 2,
		L"呐", 1,
		L"呐呐呐", 1,
		L"呐呐呐qwq", 1,
		L"诶嘿嘿", 1,
		L"诶嘿嘿qwq", 1,
		L"喵喵喵", 1,
		L"嗷嗷", 1,
		L"汪汪", 1,
		L"小fafa给你 🌸", 1,
};


int CmdMsg_qwq_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	int TotChance = 0;
	for (int i = 0; i < _countof(QwQMessage); i++)
	{
		TotChance += QwQMessage[i].Chance;
	}

	int rand_num = rand() % TotChance;

	for (int i = 0; i < _countof(QwQMessage); i++)
	{
		if (rand_num < QwQMessage[i].Chance)
		{
			SendBackMessage(boitSession, QwQMessage[i].Msg);
			break;
		}
		rand_num -= QwQMessage[i].Chance;
	}

	return 0;
}