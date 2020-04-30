#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"
#include"DirManagement.h"

WCHAR* AvaliableFunc[] = { L"RunCode", L"QueryOIer", L"QueryLuoguProblem" };

int HavePermissionToAdmin(pBOIT_SESSION boitSession)
{
	BOIT_GROUPMEMBER_INFO GroupMemberInfo;
	RetrieveGroupMemberInfo(boitSession, TRUE, &GroupMemberInfo);
	switch (GroupMemberInfo.ManageLevel)
	{
	case 2:
	case 3:
		return TRUE;
	default:
		return FALSE;
	}
}

int AdminSendHelp(pBOIT_SESSION boitSession)
{

	return 0;
}

BOOL GroupEnableDisableFunc(long long GroupID, WCHAR FuncName[], BOOL bEnable)
{
	for (int i = 0; i < _countof(AvaliableFunc); i++)
	{
		if (_wcsicmp(FuncName, AvaliableFunc[i]) == 0)
		{
			WCHAR FullPrivFileName[MAX_PATH + 2] = L"Token\\Privilege";
			wcscat_s(FullPrivFileName, MAX_PATH, FuncName);
			if (bEnable)
			{
				PerGroupCreateFileIfNExist(GroupID, FullPrivFileName);
				//TODO: 错误检查。上面这个函数可能会在文件已经存在的情况下反复创建（或是文件不在反复删除）时候返回失败。进行改进。
			}
			else
			{
				PerGroupDeleteFile(GroupID, FullPrivFileName);
				//TODO: 错误检查。上面这个函数可能会在文件已经存在的情况下反复创建（或是文件不在反复删除）时候返回失败。进行改进。
			}
			return TRUE;
		}
	}
	return FALSE;
}


int CmdMsg_admin_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if (GetBOITSessionType(boitSession) != BOITSESS_TYPE_GROUP)
	{
		SendBackMessage(boitSession, L"请在群聊中使用该指令");
		return 0;
	}

	if (!HavePermissionToAdmin(boitSession))
	{
		SendBackMessage(boitSession, L"您不是本群的群主或管理员，无权使用该指令");
		return 0;
	}


	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int OrderStrlen = GetCmdParamLen(Msg + ParamLen + SpaceLen);

	if (OrderStrlen >= 64)
	{
		SendBackMessage(boitSession, L"无法识别指令。输入#admin help查看帮助");
		return 0;
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"输入#admin help查看帮助");
		return 0;
	}

	WCHAR* OrderStr[] = { L"enable",L"disable",L"list", L"help" };
	int iMatch;
	for (iMatch = 0; iMatch < _countof(OrderStr); iMatch++)
	{
		if (OrderStrlen == wcslen(OrderStr[iMatch]) && (_wcsnicmp(Msg + ParamLen + SpaceLen, OrderStr[iMatch], OrderStrlen) == 0))
		{
			break;
		}
	}

	switch (iMatch)
	{
	case 0: // enable
	case 1: // disable
	{
		WCHAR* OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
		OrderBase += GetCmdSpaceLen(OrderBase);
		int QueryStrlen = GetCmdParamLen(OrderBase);
		if (QueryStrlen >= 64)
		{
			SendBackMessage(boitSession, L"未找到指定功能");
		}
		else if (QueryStrlen == 0)
		{
			SendBackMessage(boitSession, L"您要启用或是禁用哪个功能？(用法 #admin enable/disable 功能名。用 #admin list 查看功能)");
		}
		else
		{
			if (GroupEnableDisableFunc(GetBOITSessionGroupID(boitSession), OrderBase, iMatch == 0))
			{
				SendBackMessage(boitSession, (iMatch == 0) ? L"已成功启用该功能" : L"已成功关闭该功能");
			}
			else
			{
				SendBackMessage(boitSession, L"未找到该功能。#admin list以查看可控制的功能列表");
			}

		}
	}
	break;
	case 2: // list
		//TODO: 改为从那个数组里生成
		SendBackMessage(boitSession, L"目前可以控制的功能：\nRunCode\nQueryOIer\nQueryLuoguProblem");
		break;
	case 3: // help
		SendBackMessage(boitSession, L"管理群聊中bot功能\n#admin enable/disbale 功能名以打开关闭功能。\n#admin list以查看可控制的功能列表。\n#admin help以查看该帮助信息。");
		break;
	default:
		SendBackMessage(boitSession, L"未找到指令。输入 #admin help查看帮助");
	}


	//这里！
	//	重新把这两条指令合并成一个
	//	#admin
	//	考虑换个好点的，直观点的名字？ manage? control? ctrl? 
	//	#admin enable xxx
	//	#admin disable xxx
	//	#admin help
	//	#admin list
	//	#admin set 某人
	//	....

	return 0;
}
