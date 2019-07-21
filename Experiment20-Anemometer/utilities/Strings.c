// File:  Strings.c
// Author: JSpicer
// Date:  10/06/17
// Purpose: Misc. string utility functions.
// Hardware:  TM4C123 Tiva board

#include "Strings.h"

// itoa:  convert n to characters in s 
// This function appeared in the first edition of Kernighan and Ritchie's The C Programming Language.
// Derived from https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
int itoa(int n, char s[])
{
	 int i, sign;

	 if ((sign = n) < 0)  /* record sign */
			 n = -n;          /* make n positive */
	 i = 0;
	 do {       /* generate digits in reverse order */
			 s[i++] = n % 10 + '0';   /* get next digit */
	 } while ((n /= 10) > 0);     /* delete it */
	 if (sign < 0)
			 s[i++] = '-';
	 s[i] = '\0';
	 reverse(s);
	 return i;
}

// Returns the lenght of a null-terminated string.
int strlen(const char* s)
{
	int length = 0;
	while (*s++) {
		length++;
	}
	return length;
}


// reverse:  reverse string s in place 
// This function appeared in the first edition of Kernighan and Ritchie's The C Programming Language 
// Derived from https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
void reverse(char s[])
{
	 int i, j;
	 char c;

	 for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
			 c = s[i];
			 s[i] = s[j];
			 s[j] = c;
	 }
}

// strncpy:  copy the source string into the destination not exceeding n characters.
char* strncpy(char *dst, const char *src, int n)
{
	int i = 0;
	for (; i < n && src[i]; i++) {
		dst[i] = src[i];
	}
	
	for (int j = i; j < n; j++) {
		dst[j] = '\0';
	}
	
	return dst;
}

// strncpy:  copy the source string into the destination not exceeding n characters.
char* strcpy(char *dst, const char *src)
{
	
	int i = 0;
	for (; src[i]; i++) {
		dst[i] = src[i];
	}
	dst[i] = '\0';
	
	return dst;
}

// strncat:  concatenate at most n characters of t to the end of s; s must be big enough 
// Derived from https://github.com/thvdburgt/KnR-The-C-Programming-Language-Solutions/Chapter 5/5-3/strcat.c 
void strncat(char *s, const char *t, int n)
{
    while (*s)              /* find end of s */
        s++;
    while (*t && n-- > 0)   /* copy at most n characters of t */
        *s++ = *t++;
    *s = '\0';
}


// isspace:  return true if the character is whitespace.
int isspace(char c)
{
	return (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c =='\f' || c == '\r');
}

// isdigit:  return true if the character is a digit.
int isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

// atoi:  convert string s to integer
// Derived from https://github.com/thvdburgt/KnR-The-C-Programming-Language-Solutions/blob/master/Chapter%205/5-6/atoi.c
int atoi(const char *s)
{
    int n, sign;
    
    while (isspace(*s))
        s++;                        /* skip whitespace */
    sign = (*s == '-') ? -1 : 1;
    if (*s == '+' || *s == '-')     /* skip sign */
        s++;
    for (n = 0; isdigit(*s); s++)
        n = 10 * n + (*s -'0');
    return sign * n;
}

// pad:  pads a string with trailing characters.
void pad(char* s, char c, int size)
{
	int sourceLength = strlen(s);
	for (int i = sourceLength; i < size - 1; i++) {
		s[i] = c;
	}
	s[size - 1] = '\0';
}

/**
 *  stm32tpl --  STM32 C++ Template Peripheral Library
 *
 *  Copyright (c) 2009-2014 Anton B. Gusev aka AHTOXA
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *  
 *  description  : convert double to string
 *
 *  Original:  https://github.com/antongus/stm32tpl/blob/master/ftoa.c
 *  Modifications:
 *  - Renamed from ftoa() to dtoa(). jspicer
 */
 
 
#define MAX_PRECISION	(10)
static const double rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
	0.0005,				// 3
	0.00005,			// 4
	0.000005,			// 5
	0.0000005,			// 6
	0.00000005,			// 7
	0.000000005,		// 8
	0.0000000005,		// 9
	0.00000000005		// 10
};

char * dtoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;

	// check precision bounds
	if (precision > MAX_PRECISION)
		precision = MAX_PRECISION;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}

	if (precision < 0)  // negative precision == automatic precision guess
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}

	// round value according the precision
	if (precision)
		f += rounders[precision];

	// integer part...
	intPart = f;
	f -= intPart;

	if (!intPart)
		*ptr++ = '0';
	else
	{
		// save start pointer
		p = ptr;

		// convert (reverse order)
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}

		// save end pos
		p1 = p;

		// reverse result
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		// restore end pos
		ptr = p1;
	}

	// decimal part
	if (precision)
	{
		// place decimal point
		*ptr++ = '.';

		// convert
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}

	// terminating zero
	*ptr = 0;

	return buf;
}
