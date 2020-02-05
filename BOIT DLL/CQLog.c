//ÃÌº”»’÷æ

#include<Windows.h>
#include"CoolQDef.h"
#include"CoolQ_State.h"
int AddLog(int priority, const char* category, const char* content)
{
	return CQ_addLog(CQAuthCode, priority, category, content);
}