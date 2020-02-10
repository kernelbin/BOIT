#pragma once
#include<Windows.h>


int RegisterRead(WCHAR* BaseDir);

int RegisterWrite(WCHAR* BaseDir);

int InitializeSettings(WCHAR* BaseDir);

int ClearSettings();
