#line 1 "utilities\\Strings.c"






#line 1 "utilities\\Strings.h"








  

int itoa(int n, char s[]);


void reverse(char s[]);


int strlen(const char* s);


char* strcpy(char *dst, const char *src);


char* strncpy(char *dst, const char *src, int n);


void strncat(char *s, const char *t, int n);


int isspace(char c);


int isdigit(char c);


int atoi(const char *s);


void pad(char* s, char c, int size);


char * dtoa(double f, char * buf, int precision);

#line 8 "utilities\\Strings.c"




int itoa(int n, char s[])
{
	 int i, sign;

	 if ((sign = n) < 0)   
			 n = -n;           
	 i = 0;
	 do {        
			 s[i++] = n % 10 + '0';    
	 } while ((n /= 10) > 0);      
	 if (sign < 0)
			 s[i++] = '-';
	 s[i] = '\0';
	 reverse(s);
	 return i;
}


int strlen(const char* s)
{
	int length = 0;
	while (*s++) {
		length++;
	}
	return length;
}





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


char* strcpy(char *dst, const char *src)
{
	
	int i = 0;
	for (; src[i]; i++) {
		dst[i] = src[i];
	}
	dst[i] = '\0';
	
	return dst;
}



void strncat(char *s, const char *t, int n)
{
    while (*s)               
        s++;
    while (*t && n-- > 0)    
        *s++ = *t++;
    *s = '\0';
}



int isspace(char c)
{
	return (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c =='\f' || c == '\r');
}


int isdigit(char c)
{
	return (c >= '0' && c <= '9');
}



int atoi(const char *s)
{
    int n, sign;
    
    while (isspace(*s))
        s++;                         
    sign = (*s == '-') ? -1 : 1;
    if (*s == '+' || *s == '-')      
        s++;
    for (n = 0; isdigit(*s); s++)
        n = 10 * n + (*s -'0');
    return sign * n;
}


void pad(char* s, char c, int size)
{
	int sourceLength = strlen(s);
	for (int i = sourceLength; i < size - 1; i++) {
		s[i] = c;
	}
	s[size - 1] = '\0';
}





























 
 
 

static const double rounders[(10) + 1] =
{
	0.5,				
	0.05,				
	0.005,				
	0.0005,				
	0.00005,			
	0.000005,			
	0.0000005,			
	0.00000005,			
	0.000000005,		
	0.0000000005,		
	0.00000000005		
};

char * dtoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;

	
	if (precision > (10))
		precision = (10);

	
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}

	if (precision < 0)  
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}

	
	if (precision)
		f += rounders[precision];

	
	intPart = f;
	f -= intPart;

	if (!intPart)
		*ptr++ = '0';
	else
	{
		
		p = ptr;

		
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}

		
		p1 = p;

		
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		
		ptr = p1;
	}

	
	if (precision)
	{
		
		*ptr++ = '.';

		
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}

	
	*ptr = 0;

	return buf;
}
