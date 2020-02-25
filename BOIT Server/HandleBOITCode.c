#include<Windows.h>
#include"VBuffer.h"
#include"APITransfer.h"

BOOL SendTextWithBOITCode(long long GroupID, long long QQID, WCHAR* Msg)
{
	//¼òµ¥HandleÒ»ÏÂ
	pVBUF SendTextBuffer;
	SendTextBuffer = AllocVBuf();

	int i = 0, j = 0;
	int OrgLen = wcslen(Msg);

	int BOITFlushMaxUse = 2;
	for (i = 0; i < OrgLen;)
	{
		if (_wcsnicmp(Msg + i, L"[BOIT:flush]", wcslen(L"[BOIT:flush]")) == 0)
		{
			if (BOITFlushMaxUse-- > 0)
			{
				AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
				((WCHAR*)(SendTextBuffer->Data))[j] = 0;

				j = 0;
				SendBackMessage(GroupID, QQID, (WCHAR*)SendTextBuffer->Data);
			}
			
			i += wcslen(L"[BOIT:flush]");
		}
		else
		{
			AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
			((WCHAR*)(SendTextBuffer->Data))[j] = Msg[i];
			i++;
			j++;
		}
	}

	if (j)
	{
		AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
		((WCHAR*)(SendTextBuffer->Data))[j] = 0;
		SendBackMessage(GroupID, QQID, (WCHAR*)SendTextBuffer->Data);
	}

	FreeVBuf(SendTextBuffer);
	return 0;
}