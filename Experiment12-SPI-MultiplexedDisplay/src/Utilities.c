// File:  Utilities.c
// Author: JS
// Date:  10/06/17
// Purpose: Misc. utility functions.
// Hardware:  TM4C123 Tiva board

#include "Utilities.h"

/* itoa:  convert n to characters in s */
/* This function appeared in the first edition of Kernighan and Ritchie's The C Programming Language */
void parseInt(int n, char s[])
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

int strlen(const char* s)
{
	int length = 0;
	while (*s++) {
		length++;
	}
	return length;
}


/* reverse:  reverse string s in place */
/* This function appeared in the first edition of Kernighan and Ritchie's The C Programming Language */
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
