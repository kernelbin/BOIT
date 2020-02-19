#pragma once
#include<Windows.h>

typedef struct __tagVBuffer
{
	DWORD Length;
	DWORD Capibility;
	PBYTE Data;
}VBUF, *pVBUF;

pVBUF AllocVBuf();
BOOL FreeVBuf(pVBUF pVBuf);
BOOL AdjustVBuf(pVBUF pVBuf, DWORD Size);
BOOL AddSizeVBuf(pVBUF pVBuf, DWORD AddSize);
DWORD VBufGetCorrectSize(DWORD Size);
