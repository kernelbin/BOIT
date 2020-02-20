#include<Windows.h>
#include<wchar.h>
BOOL RemoveCQEscapeChar(WCHAR* Text)
{
	int i = 0, j = 0;
	int len = wcslen(Text);
	WCHAR EscapeCharList[][6] =					 { L"&amp;", L"&#91;", L"&#93;",L"&#44;" };
	WCHAR EscapeChar[_countof(EscapeCharList)] = { L'&',	 L'[',	   L']',    L','	 };
	while (i < len)
	{
		BOOL bMatch = 0;
		for (int k = 0; k < _countof(EscapeCharList); k++)
		{
			if (wcsncmp(Text + i, EscapeCharList[k], wcslen(EscapeCharList[k])) == 0)
			{
				Text[j++] = EscapeChar[k];
				i += wcslen(EscapeCharList[k]);
				bMatch = 1;
				break;
			}
		}
		if (!bMatch)
		{
			Text[j++] = Text[i++];
		}
	}
	Text[j++] = 0;
	return 0;
}