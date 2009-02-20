#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

void scroll(void);
void move_csr(void);
void cls(void);
void putch(unsigned char c);
void puts(unsigned char *text);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video(void);
