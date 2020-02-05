#include<Windows.h>
#include"Global.h"
#include"CQAPITransfer.h"

//生命周期

int AppInitialize()//初始化时会被执行
{
	return 0;
}

int AppFinialize()//结束时会被执行
{
	return 0;
}

int AppEnabled()//启用时执行（如果初始化时是启用的，会在AppInitialize后执行一次）
{
	return 0;
}

int AppDisabled()//禁用时执行（如果结束时是启用的，会在AppFinialize前执行一次，这点和cq原生函数不一样）
{
	return 0;
}



int HandlePrivateMessage(int subType, int msgId, long long fromQQ, const char* msg, int font)
{
	return 0;
}


int HandleGroupMessage(int subType, int msgId, long long fromGroup, long long fromQQ, const char * fromAnonymous, const char * msg, int font)
{
	if (msg[0] == '#')
	{
		SendGroupMessage(fromGroup, "正在施工中🔨\nGithub地址:https://github.com/kernelbin/BOIT \n快来star！\n有什么好的idea也欢迎来提issue！qwq我会认真看的");
	}
	return 0;
}
