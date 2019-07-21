#ifndef STRINGS_H
#define STRINGS_H

// File:  Strings.h
// Author: JSpicer
// Date:  10/06/17
// Purpose: Misc. string utility functions.
// Hardware:  TM4C123 Tiva board
  
// itoa:  convert n to characters in s.
int itoa(int n, char s[]);

// reverse:  reverse string s in place.
void reverse(char s[]);

// strlen: counts the number of s. 
int strlen(const char* s);

// strcpy:  copy the source string into the destination.
char* strcpy(char *dst, const char *src);

// strncpy:  copy the source string into the destination not exceeding n characters. 
char* strncpy(char *dst, const char *src, int n);

// strncat:  concatenate at most n characters of t to the end of s; s must be big enough 
void strncat(char *s, const char *t, int n);

// isspace:  return true if the character is whitespace. 
int isspace(char c);

// isdigit:  return true if the character is a digit. 
int isdigit(char c);

// atoi:  convert string s to integer 
int atoi(const char *s);

// pad:  pads a string with trailing characters.
void pad(char* s, char c, int size);

// dtoa:  convert double to string
char * dtoa(double f, char * buf, int precision);

#endif
