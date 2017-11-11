#include "strutils.h"
#include <string.h>

unsigned int str_equal(const char* str1, const char* str2)
{
	while (*str1 == *str2++)
	{
		if (!*str1++)
			return 1;
	}
	return 0;
}

void str_cpy(char* dest, const char* source, size_t size)
{
	if (source)
	{
		while ((--size) && (*source))
			*dest++ = *source++;
	}
	*dest = 0;
}

void str_cat(char* dest, const char* source, size_t size)
{
	if (source)
	{
		while ((--size) && (*dest))
			*dest++;

		while ((--size) && (*source))
			*dest++ = *source++;
	}
	*dest = '\0';
}

size_t numtok(const char* text, char delim)
{
	size_t ret = 0;
	size_t len = strlen(text);
	int delimIsLast = 0;

	if (text)
	{
		while ((--len) && (*text))
		{
			if (*text++ == delim)
			{
				if (*text)
					++ret;
				else
					delimIsLast = 1;
			}
		}

		if (!delimIsLast)
			++ret;
	}

	return ret;
}

char* safe_strtok(char* s, const char delim, char** last)
{
	char* tok;
	int c;

	if ((!s) && (!(s = *last)))
		return 0;

cont:
	c = *s++;

	if (c == delim)
		goto cont;

	if (!c)
	{
		*last = 0;
		return 0;
	}

	tok = s - 1;

	for (;; )
	{
		c = *s++;

		if ((c == delim) || (!c))
		{
			if (!c)
				s = 0;
			else
				s[-1] = 0;
			*last = s;

			return tok;
		}
	}
}

unsigned int isnum(const char* str)
{
	const char* s = str;

	if ((!s) || (!*s))
		return 0;

	// Unroll the for loop for efficiency (less cache misses)
	for (;;)
	{
		if (!*s) break; if (*s < '0' || *s > '9') return 0; ++s;
		if (!*s) break; if (*s < '0' || *s > '9') return 0; ++s;
		if (!*s) break; if (*s < '0' || *s > '9') return 0; ++s;
		if (!*s) break; if (*s < '0' || *s > '9') return 0; ++s;
	}

	return 1;
}

unsigned char hex_to_byte(const char c)
{
	switch (c)
	{
	case '0': return 0x0;
	case '1': return 0x1;
	case '2': return 0x2;
	case '3': return 0x3;
	case '4': return 0x4;
	case '5': return 0x5;
	case '6': return 0x6;
	case '7': return 0x7;
	case '8': return 0x8;
	case '9': return 0x9;
	case 'A': return 0xA;
	case 'B': return 0xB;
	case 'C': return 0xC;
	case 'D': return 0xD;
	case 'E': return 0xE;
	case 'F': return 0xF;
	}

	return 0;
}
