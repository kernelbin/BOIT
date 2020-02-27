#include<Windows.h>
#include<stdio.h>
#include"EstablishConn.h"
#include"Global.h"
#include"RecvEventHandler.h"
#include"RegisterRW.h"
#include"DirManagement.h"
#include"Settings.h"
#include<conio.h>
#include"CommandManager.h"
#include<Shlwapi.h>
#include"SimpleSandbox.h"
#include<process.h>
#include"MessageWatch.h"


BOOL StartInputThread();

unsigned __stdcall HandleInputThread(LPVOID Args);

BOOL ConsoleQueryYesNo(char* QueryText);


int main()
{
	puts("BOIT Server正在启动\n");

	InitializeSandbox(2, 2);

	InitializeMessageWatch();
	InitializeCommandManager();

	RegisterInlineCommand();//注册所有内置指令

	//读取注册表，检查配置
	switch (RegisterRead(GetBOITBaseDir()))
	{
	case SETTINGS_LOADED:
		puts("注册表加载成功\n");
		InitBOITDirVar();
		break;
	case SETTINGS_ERROR:
		if (ConsoleQueryYesNo("注册表加载失败，是否清空设置并初始化 ?") == FALSE)
		{
			puts("按任意键退出程序");
			_getch();
			return 0;
		}
		//清理注册表
		if (ClearSettings() == SETTINGS_ERROR)
		{
			puts("清空设置失败，按任意键退出程序");
			_getch();
			return 0;
		}
		//fall
	case SETTINGS_NOT_FOUND:
	{
		puts("欢迎使用BOIT qwq\n");
		char InBaseDir[MAX_PATH + 4] = { 0 };
		while (1)
		{
			puts("请输入BOIT目录（无需引号，完整输入一行后换行）");
			scanf_s("%[^\n]", &InBaseDir, MAX_PATH);
			getchar();//读掉 \n
			if (IsPathDirA(InBaseDir) == TRUE)
			{
				PathSimplifyA(InBaseDir);
				PathAddBackslashA(InBaseDir);
				break;
			}
			puts("这真的是一个目录吗？");
		}

		printf("将在目录 %s 初始化BOIT\n", InBaseDir);
		PathAppendA(InBaseDir, "BOIT\\");

		
		MultiByteToWideChar(CP_ACP, 0, InBaseDir, -1, GetBOITBaseDir(), MAX_PATH);
		
		if (InitializeSettings(GetBOITBaseDir()) != SETTINGS_INITIALIZED)
		{
			//Oops
			puts("初始化注册表失败，按任意键退出程序");
			_getch();
			return 0;
		}
		else
		{
			//初始化目录等
			//TODO: 检查是不是空目录
			InitBOITDirVar();

			InitBOITDir();
		}
		break;
	}
		
	}

	bBOITRemove = 0;//是否卸载程序标识

	BroadcastCommandEvent(EC_CMDLOAD, 0, 0);

	InitServerState();
	InitSendEventDispatch();;
	InitEstablishConn();
	TryEstablishConn();

	puts("连接成功！\n");
	
	StartInputThread(); // 启动控制台输入线程
	
	StartRecvEventHandler();

	WaitForSingleObject(hEventServerStop, INFINITE);

	//清理工作
	BroadcastCommandEvent(EC_CMDFREE, 0, 0);
	FinalizeCommandManager();
	FinalizeMessageWatch();
	FinalizeSandbox();

	//检查是否卸载
	if (bBOITRemove)
	{
		puts("清理中qaq");
		if (ClearSettings() == SETTINGS_ERROR)
		{
			puts("清理注册表失败，可能是权限不足导致的。请手动清理注册表项。按任意键退出程序");
			_getch();
			return 0;
		}
		if (RemoveDirIfExist(GetBOITBaseDir()) == FALSE)
		{
			puts("清理BOIT目录失败，可能是权限不足导致的。请手动清理文件夹。按任意键退出程序");
			_getch();
			return 0;
		}

		puts("BOIT注册表项和文件夹已从您的计算机上移除。");
		puts("CoolQ插件和该程序本身不会自动移除，如果需要请手动删除他们。");
		puts("感谢使用！按任意键退出程序");
		_getch();
		return 0;
	}
	else
	{
		puts("按任意键退出程序");
		_getch();
	}
	return 0;
}

BOOL ConsoleQueryYesNo(char * QueryText)
{
	while (1)
	{
		printf(QueryText);
		printf(" (y/n)\n");
		char Answer[128] = { 0 };
		scanf_s("%[^\n]", &Answer, _countof(Answer) - 1);
		getchar();//读掉那个换行
		if (strcmp(Answer, "y") == 0 ||
			strcmp(Answer, "Y") == 0 ||
			strcmp(Answer, "yes") == 0 ||
			strcmp(Answer, "Yes") == 0
			)
		{
			return TRUE;
		}
		else if (strcmp(Answer, "n") == 0 ||
			strcmp(Answer, "N") == 0 ||
			strcmp(Answer, "no") == 0 ||
			strcmp(Answer, "No") == 0
			)
		{
			return FALSE;
		}
	}
	return FALSE;
}

BOOL StartInputThread()
{
	_beginthreadex(0, 0, HandleInputThread, 0, 0, 0);
	return 0;
}

unsigned __stdcall HandleInputThread(LPVOID Args)
{
	while (1)
	{
		char InputCommand[128];
		printf("> ");
		int bMatch = scanf_s("%[^\n]", InputCommand, _countof(InputCommand));
		getchar();
		if (bMatch == 1)
		{
			if (_strcmpi(InputCommand, "stop") == 0)
			{
				puts("Goodbye");
				SetEvent(hEventServerStop);
				return 0;
			}
			else if (_strcmpi(InputCommand, "help") == 0)
			{
				puts("帮助信息：\n\nstop:  关闭BOIT\nremove:  卸载BOIT\nhelp:  显示帮助信息");
			}
			else if (_strcmpi(InputCommand, "remove") == 0)
			{
				if (ConsoleQueryYesNo("这将关闭BOIT从注册表中清除所有BOIT配置项，并移除BOIT目录。确定吗？") == TRUE)
				{
					bBOITRemove = TRUE;
					SetEvent(hEventServerStop);
					puts("关闭BOIT中");
					return 0;
					//main函数退出的时候会检测bBOITRemove
				}
			}
		}
	}
}

