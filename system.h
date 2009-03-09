#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define __noreturn__		__attribute__((noreturn))
#define __printf__(a, b)	__attribute__((format(printf, a, b)))

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

void panic(const char *msg);

void gdt_install(void);

void idt_install(void);
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
		unsigned char flags);

void isrs_install(void);
void register_isr(unsigned irq, void (*handler)(struct regs *r));
static inline void register_irq(unsigned irq, void (*handler)(struct regs *r))
{
	register_isr(irq+32, handler);
}

void init_pit(void);
void timer_phase(int hz);
unsigned long gettimeofday(void);

void init_kbd(void);
