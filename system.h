typedef unsigned size_t;

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

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
void isrs_install(void);

struct regs {
	unsigned int gs, fs, es, ds;      /* pushed the segs last */
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
	unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
	unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */
};

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
				unsigned char flags);

