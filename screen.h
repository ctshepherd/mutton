#ifndef SCREEN_H
#define SCREEN_H

#include "type.h"

void scroll(void);
void move_csr(void);
void cls(void);

void putch(char c);
static inline void puts(const char *text)
{
	while (*text)
		putch(*text++);
}
int __printf__(1, 2) printf(const char *format, ...);

void settextcolor(unsigned char forecolor, unsigned char backcolor);

#endif
