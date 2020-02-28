#include<Windows.h>

#define CP_GB18030 54936

WCHAR* StrConvMB2WC(UINT CodePage, LPCCH MbStr, int cbMbStrlen, int* cchWcLen); //转换字符串并默认补一个 \0在结尾。长度传入-1则自动计算长度，内存需要手动free

char* StrConvWC2MB(UINT CodePage, LPCWCH WcStr, int cchWcStrlen, int* cbMbLen); //转换字符串并默认补一个 \0在结尾。长度传入-1则自动计算长度，内存需要手动free
