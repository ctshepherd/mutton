#include "string.h"

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		int x = *s1 - *s2;
		if (x)
			return x;
		s1++;
		s2++;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (*s1 && *s2 && n) {
		int x = *s1 - *s2;
		if (x)
			return x;
		s1++;
		s2++;
		n--;
	}
	return 0;
}

size_t strlen(const char *s)
{
	const char *o = s;
	while (*s++);
	return s-o;
}

char *strcpy(char *dest, const char *src)
{
	char *o = dest;
	do {
		*dest++ = *src;
	} while (*src++);
	return o;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *o = dest;
	do {
		*dest++ = *src;
	} while (*src++ && n--);

	return o;
}

static inline char _case_transform(char s)
{
	if ('a' <= s && s <= 'z')
		return s+'A'-'a';
	return s;
}

int strcasecmp(const char *s1, const char *s2)
{
	int tmp1, tmp2;
	for ( ; *s1; s1++, s2++) {
		int x;
		tmp1 = _case_transform(*s1);
		tmp2 = _case_transform(*s2);
		x = tmp1 - tmp2;
		if (x)
			return x;
	}
	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	char *_d = dest;
	const char *_s = src;
	while (n--)
		*_d++ = *_s++;
	return dest;
}

void *memset(void *d, int c, size_t n)
{
	char *_d = d;
	while (n--)
		*_d++ = c;
	return d;
}

unsigned short *memsetw(unsigned short *d, unsigned short c, size_t n)
{
	unsigned short *_d = d;
	while(n--)
		*_d++ = c;
	return d;
}
