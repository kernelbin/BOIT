#include<Windows.h>

#define BOITCODE_MAX_PARAM 8



// flag是一个DWORD值

#define SWBCFLAG_MAX_FLUSH_OFFSET 16
#define SWBC_PARSE_AT		0x01
#define SWBC_PARSE_AT_ALL	0x02 
#define SWBC_PARSE_IMG_URL	0x04
#define SWBC_PARSE_IMG_FILE	0x08

#define SWBC_PARSE_FLUSH(MaxTime) ((MaxTime)<<(SWBCFLAG_MAX_FLUSH_OFFSET))


typedef struct __tagBOITCode
{
	WCHAR* TypeStr;

	int ParamNum;

	WCHAR* Key[BOITCODE_MAX_PARAM];
	WCHAR* Value[BOITCODE_MAX_PARAM];

}BOITCODEINFO, * pBOITCODEINFO;

BOOL SendTextWithBOITCode(pBOIT_SESSION boitSession, WCHAR* Msg, DWORD flags);



