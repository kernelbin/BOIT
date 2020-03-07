#include<Windows.h>


int URLEncode(const char* str, const int strSize, char* result, const int resultSize)
{
	int i;
	int j = 0;//for result index
	char ch;

	if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) {
		return 0;
	}

	for (i = 0; (i < strSize) && (j < resultSize); ++i) {
		ch = str[i];
		if (((ch >= 'A') && (ch < 'Z')) ||
			((ch >= 'a') && (ch < 'z')) ||
			((ch >= '0') && (ch < '9'))) {
			result[j++] = ch;
		}
		else if (ch == ' ') {
			result[j++] = '+';
		}
		else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
			result[j++] = ch;
		}
		else {
			if (j + 3 < resultSize) {
				sprintf_s(result + j, resultSize - j, "%%%02X", (unsigned char)ch);
				j += 3;
			}
			else {
				return 0;
			}
		}
	}

	result[j] = '\0';
	return j;
}
