#include<Windows.h>
#include<malloc.h>
#include"VBuffer.h"


pVBUF AllocVBuf()
{
	pVBUF pVBuf= malloc(sizeof(VBUF));
	memset(pVBuf, 0, sizeof(VBUF));

	return pVBuf;
}

BOOL FreeVBuf(pVBUF pVBuf)
{
	if (pVBuf->Data)
	{
		free(pVBuf->Data);
	}
	free(pVBuf);
	return TRUE;
}

BOOL AdjustVBuf(pVBUF pVBuf, DWORD Size)
{
	pVBuf->Length = Size;
	pVBuf->Capibility = VBufGetCorrectSize(Size);
	
	if (pVBuf->Capibility)
	{
		if (!pVBuf->Data)
		{
			pVBuf->Data = malloc(pVBuf->Capibility);
		}
		else
		{
			pVBuf->Data = realloc(pVBuf->Data, pVBuf->Capibility);
		}
	}
	else
	{
		free(pVBuf->Data);//free支持参数NULL
		pVBuf->Data = NULL;
	}
	

	return 0;
}

BOOL AddSizeVBuf(pVBUF pVBuf,DWORD AddSize)
{
	return AdjustVBuf(pVBuf, pVBuf->Length + AddSize);
}

DWORD VBufGetCorrectSize(DWORD Size)
{
	int iBit = 0;
	for (DWORD iBit = 1 << (sizeof(DWORD) * 8 - 1); iBit; iBit >>= 1)
	{
		if (iBit & Size)
		{
			if (iBit == Size)
			{
				return iBit;
			}
			else
			{
				return iBit << 1;
			}
			
		}
	}
	return 0;
}


BOOL VBufferAppendStringW(pVBUF VBuffer, WCHAR* String)
{
	int OrgLen = VBuffer->Length;
	int cchwcLen = wcslen(String);
	AddSizeVBuf(VBuffer, cchwcLen * sizeof(WCHAR));
	memcpy(VBuffer->Data + OrgLen, String, cchwcLen * sizeof(WCHAR));
	return TRUE;
}

