#include<Windows.h>

#define BOITCODE_MAX_PARAM 8
typedef struct __tagBOITCode
{
	WCHAR* TypeStr;

	int ParamNum;

	WCHAR* Key[BOITCODE_MAX_PARAM];
	WCHAR* Value[BOITCODE_MAX_PARAM];

}BOITCODEINFO, * pBOITCODEINFO;

BOOL SendTextWithBOITCode(pBOIT_SESSION boitSession, WCHAR* Msg);
