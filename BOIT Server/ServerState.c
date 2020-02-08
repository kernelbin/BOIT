#include<Windows.h>
#include"Global.h"

//现在仅仅是记录server是否正在运行中，以后可能加入忙碌程度，正在处理等


int InitServerState()
{
	hEventServerStop = CreateEvent(0, 1, 0, 0);
	return 0;
}

int ServerStop()
{
	SetEvent(hEventServerStop);
	return 0;
}
