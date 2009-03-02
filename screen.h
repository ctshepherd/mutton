#ifndef SCREEN_H
#define SCREEN_H

void scroll(void);
void move_csr(void);
void cls(void);

void putch(char c);
static inline void puts(const char *text)
{
	while (*text)
		putch(*text++);
}
int printf(const char *format, ...);

void settextcolor(unsigned char forecolor, unsigned char backcolor);

#endif
