#pragma once
#include<Windows.h>

typedef struct __tagVBuffer
{
	PBYTE Data;
	DWORD Length;
	DWORD Capibility;
}VBUF, *pVBUF;

pVBUF AllocVBuf();
BOOL FreeVBuf(pVBUF pVBuf);
BOOL AdjustVBuf(pVBUF pVBuf, DWORD Size);
BOOL AddSizeVBuf(pVBUF pVBuf, DWORD AddSize);
DWORD VBufGetCorrectSize(DWORD Size);

BOOL VBufferAppendStringW(pVBUF VBuffer, WCHAR* String);