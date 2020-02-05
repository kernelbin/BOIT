#include<Windows.h>
#include"ObjectName.h"
#include"EstablishConn.h"
#
//这个文件的代码是共享的，用于建立DLL和Server之间的连接


#define JUDGE_OBJ(Obj, iRet) \
if(Obj){\
if(GetLastError() == ERROR_ALREADY_EXISTS){\
if(iRet == -2 || iRet == 1){iRet = 1;}else{__leave;}\
}\
else{\
if(iRet == -2 || iRet == 0){iRet = 0;}else{__leave;}\
}\
}\
else{\
__leave;\
}


int TryEstablishConn()//return -1代表失败 0代表成功创建对象并等待到连接 1代表成功连接
{
	int iRet = -2;
	__try
	{
		hEventConnect = CreateEvent(0, 0, 0, GET_OBJ_NAME(EVENT_CONNECT));
		JUDGE_OBJ(hEventConnect, iRet);


	}
	__finally
	{
		switch(iRet)
		{
		case -1:
			CloseHandle(hEventConnect);
			break;
		case 0:
			WaitForSingleObject(hEventConnect, INFINITE);
			break;
		case 1:
			SetEvent(hEventConnect);
			break;
		}
		
	}
	
	return iRet;
}


