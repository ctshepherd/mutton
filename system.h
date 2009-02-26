#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

void gdt_install(void);
void idt_install(void);
void isrs_install(void);

struct regs {
	/* pushed the segs last */
	unsigned int gs, fs, es, ds;
	/* pushed by 'pusha' */
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	/* our 'push byte #' and ecodes do this */
	unsigned int int_no, err_code;
	/* pushed by the processor automatically */
	unsigned int eip, cs, eflags, useresp, ss;
};

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
				unsigned char flags);
