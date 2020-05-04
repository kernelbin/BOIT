#include<Windows.h>

int GetLineLen(WCHAR* String);

int GetLineSpaceLen(WCHAR* String);

int GetLineFeedLen(WCHAR* String);

int GetCmdSpaceLen(WCHAR* String);

int GetCmdParamLen(WCHAR* String);

int GetCmdParamWithEscapeLen(WCHAR* String);

int CmdParamUnescape(WCHAR* String, WCHAR* UnescapeStr);

int GetBOITCodeParamWithEscapeLen(WCHAR* String);

int BOITCodeParamUnescape(WCHAR* String, WCHAR* UnescapeStr);
