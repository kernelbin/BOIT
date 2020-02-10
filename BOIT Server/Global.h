#pragma once
#include<Windows.h>

SRWLOCK SendLock;

HANDLE hEventServerStop;

int InitServerState();
int ServerStop();

int InitSendEventDispatch();

