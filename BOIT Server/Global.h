#pragma once
#include<Windows.h>

SRWLOCK SendLock;

HANDLE hEventServerStop;

BOOL bBOITRemove; // «∑Ò–∂‘ÿ≥Ã–Ú

int InitServerState();
int ServerStop();

int StartSendEventHandler();

int InitSendEventDispatch();

