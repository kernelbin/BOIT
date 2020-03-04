#include<Windows.h>
#include"Corpus.h"

WCHAR* Corpus_CuteMessage[] = {
		L"pwp",
		L"qwq",
		L"owo",
		L"awa",
		L"/w\\",
		L"QwQ",
		L"/qwq\\",
		L"ヾ(≧▽≦*)o",
		L"`(*>﹏<*)′",
		L"(*≧▽≦)o☆" ,
		L"(*/ω＼*)",
		L"(。・∀・)ノ"
};

WCHAR * Corpus_NoPrivilegeMessage[] = {
		L"Oops... 您没有适当的权限进行操作",
		L"您没有权限执行该指令",
		L"不要 我才不听你的话 哼~",
		L"不要这样嘛...",
		L"刚刚你说啥...？ 我没听清？",
		L"你坏坏 我不听你的",
		L"我不听你的 请你吃桃子",
		L"爪巴",
		L"爬",
		L"有内鬼，终止执行指令。",
		L"才不要呢~",
		L"You are not in the sudoers file. This incident will be reported.",
		L"Unable to do this, are you root?"
};


WCHAR* Corpus_WhereIsInputMessage[] = {
		L"我好饿，喂给我的输入数据去哪里了qaq",
		L"好饿.....",
		L"输入数据去哪里了qaq",
		L"输入数据呢qwq",
		L"qaq?",
		L"不输入拉倒",
		L"不输入拉倒，不等你了"
};


WCHAR* Corpus_CodeNotFoundMessage[] = {
		L"诶？代码找不到了诶~",
		L"这里没有你的代码...",
		L"你的代码去哪里了诶...？",
		L"找不到你的代码了，要不先存一份代码？"
};

//语料库

WCHAR* Corpus_Cute()
{
	return Corpus_CuteMessage[rand() % _countof(Corpus_CuteMessage)];
}


WCHAR* Corpus_NoPrivilege()
{
	return Corpus_NoPrivilegeMessage[rand() % _countof(Corpus_NoPrivilegeMessage)];
}


WCHAR* Corpus_WhereIsInput()
{
	return Corpus_WhereIsInputMessage[rand() % _countof(Corpus_WhereIsInputMessage)];
}



WCHAR* Corpus_CodeNotFound()
{
	return Corpus_CodeNotFoundMessage[rand() % _countof(Corpus_CodeNotFoundMessage)];
}