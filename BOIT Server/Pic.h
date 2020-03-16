#include<Windows.h>

#ifdef __cplusplus
extern "C" BOOL SaveHDCToFile(HDC hDC, LPRECT lpRect, WCHAR FilePath[]);
extern "C" BOOL InitGDIPlus();
extern "C" BOOL CleanupGDIPlus();
#else
BOOL SaveHDCToFile(HDC hDC, LPRECT lpRect, WCHAR FilePath[]);
BOOL InitGDIPlus();
BOOL CleanupGDIPlus();
#endif