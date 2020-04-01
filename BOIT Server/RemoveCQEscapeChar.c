#include<Windows.h>
#include<wchar.h>


UINT BytesToUTF16LE(UINT nCodePage, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    UINT nchLen;

    switch (nCodePage)
    {
    case 1200: // UTF-16LE
    case 1201: // UTF-16BE
    {
        if ((!lpMultiByteStr) || (cbMultiByte < 0) || (cchWideChar < 0))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }

        cbMultiByte /= 2;
        nchLen = cbMultiByte;

        if (lpWideCharStr)
        {
            if (cchWideChar < nchLen)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return 0;
            }

            if (nCodePage == 1200)
                CopyMemory(lpWideCharStr, lpMultiByteStr, nchLen * 2);
            else
            {
                PUINT16 pCodeUnits = (PUINT16)lpMultiByteStr;

                for (int i = 0; i < cbMultiByte; ++i)
                {
                    lpWideCharStr[i] = (WCHAR)(
                        ((pCodeUnits[i] << 8) & 0xFF00) |
                        ((pCodeUnits[i] >> 8) & 0x00FF)
                        );
                }
            }
        }

        SetLastError(0);
        break;
    }

    case 12000: // UTF-32LE
    case 12001: // UTF-32BE
    {
        if ((!lpMultiByteStr) || (cbMultiByte < 0) || (cchWideChar < 0))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }

        PUINT32 pCodePoints = (PUINT32)lpMultiByteStr;
        cbMultiByte /= 4;

        nchLen = 0;

        for (int i = 0; i < cbMultiByte; ++i)
        {
            UINT32 CodePoint = pCodePoints[i];
            if (nCodePage == 12001)
            {
                CodePoint = (
                    ((CodePoint >> 24) & 0x000000FF) |
                    ((CodePoint >> 8) & 0x0000FF00) |
                    ((CodePoint << 8) & 0x00FF0000) |
                    ((CodePoint << 24) & 0xFF000000)
                    );
            }

            if (CodePoint < 0x10000)
            {
                if (lpWideCharStr)
                {
                    if (cchWideChar < 1)
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return 0;
                    }

                    *lpWideCharStr++ = (WCHAR)(CodePoint & 0xFFFF);
                    --cchWideChar;
                }

                ++nchLen;
            }
            else if (CodePoint <= 0x10FFFF)
            {
                if (lpWideCharStr)
                {
                    if (cchWideChar < 2)
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return 0;
                    }

                    CodePoint -= 0x10000;
                    *lpWideCharStr++ = (WCHAR)(0xD800 + ((CodePoint >> 10) & 0x3FF));
                    *lpWideCharStr++ = (WCHAR)(0xDC00 + (CodePoint & 0x3FF));
                    cchWideChar -= 2;
                }

                nchLen += 2;
            }
            else
            {
                SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                return 0;
            }
        }

        SetLastError(0);
        break;
    }

    default:
        nchLen = MultiByteToWideChar(nCodePage, 0, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
        break;
    }

    return nchLen;
}


BOOL RemoveCQEscapeChar(WCHAR* Text)
{
	int i = 0, j = 0;
	int len = wcslen(Text);
	WCHAR EscapeCharList[][6] =					 { L"&amp;", L"&#91;", L"&#93;",L"&#44;" };
	WCHAR EscapeChar[_countof(EscapeCharList)] = { L'&',	 L'[',	   L']',    L','	 };
	while (i < len)
	{
		BOOL bMatch = 0;

        //检查是否是表情 [CQ:emoji,id=数字]
        int EmojiUnicode;
        if (swscanf_s(Text + i, L"[CQ:emoji,id=%d]", &EmojiUnicode) == 1)
        {
            //got! 是表情
            UINT32 EmojiStr[2 + 1] = { 0 };
            EmojiStr[0] = EmojiUnicode;
            WCHAR wcEmoji[4 + 1] = { 0 };//最糟糕情况下8字节，也就是4个
            BytesToUTF16LE(12000 /*utf-32*/, (LPCSTR)EmojiStr, sizeof(EmojiStr), wcEmoji, sizeof(wcEmoji));

            for (int x = 0; wcEmoji[x];)
            {
                Text[j++] = wcEmoji[x++];
            }

            WCHAR* CQEmojiEnd = wcschr(Text + i, L']');
            i = CQEmojiEnd - Text + 1;
            continue;
        }

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