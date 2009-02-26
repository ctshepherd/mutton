#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

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

