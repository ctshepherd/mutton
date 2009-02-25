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

void settextcolor(unsigned char forecolor, unsigned char backcolor);

static inline void init_video(void)
{
	cls();
}

#endif
