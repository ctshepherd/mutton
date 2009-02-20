typedef unsigned size_t;

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

unsigned char inportb(unsigned short _port);
void outportb(unsigned short _port, unsigned char _data);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

void scroll(void);
void move_csr(void);
void cls(void);
void putch(unsigned char c);
void puts(unsigned char *text);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video(void);

void gdt_install(void);
void idt_install(void);
