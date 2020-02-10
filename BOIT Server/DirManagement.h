#pragma once
#include<Windows.h>
BOOL SetBOITBaseDir(WCHAR* Dir);

WCHAR* GetBOITBaseDir();

WCHAR BOITBaseDir[MAX_PATH];