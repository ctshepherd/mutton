typedef unsigned size_t;

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

unsigned char inportb(unsigned short port);
unsigned short inportw(unsigned short port);
void outportb(unsigned short port, unsigned char data);
void outportw(unsigned short port, unsigned short data);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

void scroll(void);
void move_csr(void);
void cls(void);
void putch(char c);
void puts(const char *text);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video(void);

void bochs_putch(char c);
void bochs_puts(const char *text);
void bochs_break(void);

void gdt_install(void);
void idt_install(void);
