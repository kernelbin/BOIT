#pragma once
#include<Windows.h>
#include"BOITInfoMaxDef.h"

//联系人（如QQ好友，群友）信息结构体
typedef struct __tagGroupMemberInfo
{
	long long GroupID;
	long long QQID;
	WCHAR NickName[BOIT_MAX_NICKLEN + 4];
	WCHAR CardName[BOIT_MAX_NICKLEN + 4];
	int Gender; // 0/男性 1/女性 255/未知
	int Age;
	WCHAR Location[BOIT_MAX_LOCATION + 4]; // 地区
	int EnterGroupTime;
	int LastActive;//最后发言时间
	WCHAR LevelName[BOIT_MAX_LOCATION + 4]; // 地区
	int ManageLevel; // 管理权限 1/成员 2/管理员 3/群主
	BOOL bBadRecord; //不良记录
	WCHAR SpecialTitle[BOIT_MAX_NICKLEN]; //专属头衔
	int SpecTitExpire;//专属头衔过期时间。-1不过期
	BOOL bAllowEditCard;//是否允许编辑群名片

}BOIT_GROUPMEMBER_INFO, * pBOIT_GROUPMEMBER_INFO;


typedef struct __tagStrangerInfo
{
	long long QQID;
	WCHAR NickName[BOIT_MAX_NICKLEN + 4];
	int Gender; // 0/男性 1/女性 255/未知
	int Age;
}BOIT_STRANGER_INFO, * pBOIT_STRANGER_INFO;


typedef struct __tagEventRecvStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;
			long long QQID;
			WCHAR AnonymousName[BOIT_MAX_NICKLEN + 4];

			int iRet;
		}GroupMsg;
		 
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;

			int iRet;
		}PrivateMsg;

		
	}u;
}EVENT_RECV, *pEVENT_RECV;


typedef struct __tagEventSendStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;

			int iRet;
		}GroupMsg;

		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;

			int iRet;
		}PrivateMsg;

		struct
		{
			long long GroupID;
			long long QQID;
			BOOL NoCache;
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;

			int iRet; // 1代表成功 0代表失败
		}GroupMemberInfo;


		struct
		{
			long long QQID;
			BOOL NoCache;
			BOIT_STRANGER_INFO StrangerInfo;

			int iRet; // 1代表成功 0代表失败
		}StrangerInfo;
	}u;
}EVENT_SEND,*pEVENT_SEND;


typedef struct __tagSharedProcessInfo
{
	DWORD pid[2];
}SHARED_PROCESS_INFO, *pSHARED_PROCESS_INFO;
