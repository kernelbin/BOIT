#include<Windows.h>
#include"EstablishConn.h"
#include"CQAPITransfer.h"
#include"BOITEventType.h"
#include<process.h>
#include"SharedMemStruct.h"
#include"Base64.h"
#include"EncodeConvert.h"

unsigned __stdcall SendEventThread(void* Args);

BOOL RetrieveGroupMemberInfoFromCQ(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo);

BOOL RetrieveStrangerInfoFromCQ(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo);

int StartSendEventHandler()
{
	_beginthreadex(NULL, 0, SendEventThread, (LPVOID)0, 0, NULL);
	
	return 0;
}


unsigned __stdcall SendEventThread(void *Args)
{
	while (1)
	{
		while (GetConnState() == 1)
		{
			if (ConnWaitForObject(hEventSendStart) == 0)
			{
				break;
			}
		
			//分类：
			switch (pSharedMemSend->EventType)
			{
			case BOIT_EVENT_SEND_PRIVATE:
			{
				char* ToSendText;
				int ccbLen;
				int iRet = 0;

				ToSendText = StrConvWC2MB(CP_GB18030, pSharedMemSend->u.PrivateMsg.Msg, -1, &ccbLen);
				
				if (ToSendText)
				{
					iRet = SendPrivateMessage(pSharedMemSend->u.PrivateMsg.QQID, ToSendText);
					free(ToSendText);
				}
				pSharedMemSend->u.PrivateMsg.iRet = iRet;
			}
				break;
			case BOIT_EVENT_SEND_GROUP:
			{
				char* ToSendText;
				int ccbLen;
				int iRet = 0;

				ToSendText = StrConvWC2MB(CP_GB18030, pSharedMemSend->u.GroupMsg.Msg, -1, &ccbLen);

				if (ToSendText)
				{
					iRet = SendGroupMessage(pSharedMemSend->u.GroupMsg.GroupID, ToSendText);
					free(ToSendText);
				}
				pSharedMemSend->u.GroupMsg.iRet = iRet;
			}
				break;
			case BOIT_EVENT_SEND_GET_GROUPMEMBER_INFO:
			{
				int iRet = RetrieveGroupMemberInfoFromCQ(pSharedMemSend->u.GroupMemberInfo.GroupID,
					pSharedMemSend->u.GroupMemberInfo.QQID,
					pSharedMemSend->u.GroupMemberInfo.NoCache,
					&(pSharedMemSend->u.GroupMemberInfo.GroupMemberInfo));
				pSharedMemSend->u.GroupMemberInfo.iRet = iRet;
			}
				break;
			case BOIT_EVENT_SEND_GET_STRANGER_INFO:
			{
				int iRet = RetrieveStrangerInfoFromCQ(pSharedMemSend->u.StrangerInfo.QQID,
					pSharedMemSend->u.StrangerInfo.NoCache,
					&(pSharedMemSend->u.StrangerInfo.StrangerInfo));
				pSharedMemSend->u.StrangerInfo.iRet = iRet;
			}
			break;
			}
			
			SetEvent(hEventSendEnd);

			if (ConnWaitForObject(hEventSendRet) == 0)
			{
				break;
			}
		}
		Sleep(1000);
	}
	
}


long long RetrieveNumber(char* Data, int* offset, int len)
{
	long long ret = 0;
	Data += (*offset);
	for (int i = 0; i < len; i++)
	{
		ret <<= 8;
		ret |= (unsigned char)Data[i];
	}
	(*offset) += len;
	return ret;
}

char* RetrieveString(char* Data, int* offset, int* len)
{
	int StringLen = (int)RetrieveNumber(Data, offset,2);
	char* RetStr = Data + (*offset);
	(*len) = StringLen;
	(*offset) +=  StringLen;
	
	if (StringLen == 0)
	{
		return 0;
	}
	
	return RetStr;
}

BOOL RetrieveGroupMemberInfoFromCQ(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo)
{
	const char * InfoStr = GetGroupMemberInfo(GroupID, QQID, NoCache);
	if (!InfoStr)
	{
		return FALSE;
	}
	int DataLen = 3 * ((strlen(InfoStr) + 3) / 4) + 6;//加6保险，我心虚
	PBYTE DecodeData;
	DecodeData = malloc(DataLen);
	if (!DecodeData)
	{
		return FALSE;
	}
	ZeroMemory(DecodeData, DataLen);
	Base64Decode(InfoStr, DecodeData);

	int offset = 0;
	int StringLen;


	long long RetrGroupID = RetrieveNumber(DecodeData, &offset, 8);
	GroupMemberInfo->GroupID = RetrGroupID;//赋值

	long long RetrQQID = RetrieveNumber(DecodeData, &offset, 8);
	GroupMemberInfo->QQID = RetrQQID;//赋值


	char* RetrNickName = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrNickName, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_NICKLEN)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrNickName, StringLen, GroupMemberInfo->NickName, wcStrlen);
			GroupMemberInfo->NickName[wcStrlen] = 0;
		}
	}
	else
	{
		GroupMemberInfo->NickName[0] = 0;
	}

	char* RetrCardName = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrCardName, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_NICKLEN)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrCardName, StringLen, GroupMemberInfo->CardName, wcStrlen);
			GroupMemberInfo->CardName[wcStrlen] = 0;
		}

	}
	else
	{
		GroupMemberInfo->CardName[0] = 0;
	}


	int RetrGender = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->Gender = RetrGender;


	int RetrAge = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->Age = RetrAge;


	char* RetrLocation = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrLocation, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_LOCATION)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrLocation, StringLen, GroupMemberInfo->Location, wcStrlen);
			GroupMemberInfo->Location[wcStrlen] = 0;
		}

	}
	else
	{
		GroupMemberInfo->Location[0] = 0;
	}


	int RetrEnterGroupTime = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->EnterGroupTime = RetrEnterGroupTime;


	int RetrLastActive = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->LastActive = RetrLastActive;


	char* RetrLevelName = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrLevelName, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_LEVELNAME)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrLevelName, StringLen, GroupMemberInfo->LevelName, wcStrlen);
			GroupMemberInfo->LevelName[wcStrlen] = 0;
		}

	}
	else
	{
		GroupMemberInfo->LevelName[0] = 0;
	}


	int RetrManageLevel = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->ManageLevel = RetrManageLevel;


	int RetrBadRecord = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->bBadRecord = RetrBadRecord;


	char* RetrSpecialTitle = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrSpecialTitle, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_NICKLEN)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrSpecialTitle, StringLen, GroupMemberInfo->SpecialTitle, wcStrlen);
			GroupMemberInfo->SpecialTitle[wcStrlen] = 0;
		}

	}
	else
	{
		GroupMemberInfo->SpecialTitle[0] = 0;
	}


	int RetrSpecTitExpire = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->SpecTitExpire = RetrSpecTitExpire;


	int RetrAlloEditCard = (int)RetrieveNumber(DecodeData, &offset, 4);
	GroupMemberInfo->bAllowEditCard = RetrAlloEditCard;

		
	free(DecodeData);
	return TRUE;
}

BOOL RetrieveStrangerInfoFromCQ(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo)
{
	const char* InfoStr = GetStrangerInfo(QQID, NoCache);
	if (!InfoStr)
	{
		return FALSE;
	}
	int DataLen = 3 * ((strlen(InfoStr) + 3) / 4) + 6;//加6保险，我心虚
	PBYTE DecodeData;
	DecodeData = malloc(DataLen);
	if (!DecodeData)
	{
		return FALSE;
	}
	ZeroMemory(DecodeData, DataLen);
	Base64Decode(InfoStr, DecodeData);

	int offset = 0;
	int StringLen;

	long long RetrQQID = RetrieveNumber(DecodeData, &offset, 8);
	StrangerInfo->QQID = RetrQQID;//赋值


	char* RetrNickName = RetrieveString(DecodeData, &offset, &StringLen);
	//赋值
	if (StringLen)
	{
		int wcStrlen = MultiByteToWideChar(CP_GB18030, 0, RetrNickName, StringLen, 0, 0);
		if (wcStrlen >= BOIT_MAX_NICKLEN)
		{
			//nmd????
			//TODO:异常处理
		}
		else
		{
			MultiByteToWideChar(CP_GB18030, 0, RetrNickName, StringLen, StrangerInfo->NickName, wcStrlen);
			StrangerInfo->NickName[wcStrlen] = 0;
		}
	}
	else
	{
		StrangerInfo->NickName[0] = 0;
	}


	int RetrGender = (int)RetrieveNumber(DecodeData, &offset, 4);
	StrangerInfo->Gender = RetrGender;


	int RetrAge = (int)RetrieveNumber(DecodeData, &offset, 4);
	StrangerInfo->Age = RetrAge;


	free(DecodeData);
	return TRUE;
}


