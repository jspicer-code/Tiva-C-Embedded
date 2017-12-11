// File:  Strings.c
// Author: JSpicer
// Date:  10/06/17
// Purpose: Misc. string utility functions.
// Hardware:  TM4C123 Tiva board

#include "Strings.h"

// itoa:  convert n to characters in s 
// This function appeared in the first edition of Kernighan and Ritchie's The C Programming Language.
// Derived from https://en.wikibooks.org/wiki/C_Programming/stdlib.h/itoa
void itoa(int n, char s[])
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
