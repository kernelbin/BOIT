#include<Windows.h>
#include"CommandProcess.h"


int GetLineLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == 0)
		{
			break;
		}
	}
	return i;
}


int GetLineSpaceLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\t')
		{
			break;
		}
	}
	return i;
}


int GetLineFeedLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\r' &&
			String[i] != L'\n')
		{
			break;
		}
	}
	return i;
}


int GetCmdSpaceLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\r' &&
			String[i] != L'\n' &&
			String[i] != L'\t')
		{
			break;
		}
	}
	return i;
}


int GetCmdParamLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] == L' ' ||
			String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}
	}
	return i;
}


int GetCmdParamWithEscapeLen(WCHAR* String)
{
	int i;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				i++;
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
		}
	}
	return i;
}


int CmdParamUnescape(WCHAR* String, WCHAR* UnescapeStr)
{
	int i = 0;
	int j = 0;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
	}
	UnescapeStr[j++] = 0;
	return i;
}


int GetBOITCodeParamWithEscapeLen(WCHAR* String)
{
	int i;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				i++;
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
			else if (String[i] == L',')
			{
				break;
			}
			else if (String[i] == L']')
			{
				break;
			}
		}
	}
	return i;
}


int BOITCodeParamUnescape(WCHAR* String, WCHAR* UnescapeStr)
{
	int i = 0;
	int j = 0;
	BOOL bInEscape = FALSE;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == L'\t' ||
			String[i] == 0)
		{
			break;
		}

		if (bInEscape)
		{
			if (String[i] == L'\"')
			{
				bInEscape = FALSE;
			}
			// L' ' is ignored
			else if (String[i] == L'\\' && String[i + 1] == L'\\')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\'')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else if (String[i] == L'\\' && String[i + 1] == L'\"')
			{
				UnescapeStr[j++] = String[i + 1];
				i++;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
		else
		{
			if (String[i] == L'\"')
			{
				bInEscape = TRUE;
			}
			else if (String[i] == L' ')
			{
				break;
			}
			else if (String[i] == L',')
			{
				break;
			}
			else if (String[i] == L']')
			{
				break;
			}
			else
			{
				UnescapeStr[j++] = String[i];
			}
		}
	}
	UnescapeStr[j++] = 0;
	return i;
}
