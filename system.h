#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

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

void stack_unwind(void);
void panic(const char *msg);
void _assert(const char *statement, const char *func, unsigned line);
#define assert(c) do {					\
	if (!(c))					\
		_assert(#c, __func__, __LINE__);	\
} while (0)

void gdt_install(void);

void idt_install(void);
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
		unsigned char flags);

typedef void (*irq_handler_t)(struct regs *r);
void isrs_install(void);
void register_isr(unsigned irq, irq_handler_t handler);
static inline void register_irq(unsigned irq, irq_handler_t handler)
{
	register_isr(irq+32, handler);
}

void init_pit(void);
void timer_phase(int hz);
unsigned long gettimeofday(void);

void init_kbd(void);

#define ALIGN(a, n)		((typeof(a))((unsigned long)a & ~((n)-1)) + (n))

#define offsetof(type, member)		__builtin_offsetof(type, member)
#define container_of(ptr, type, member) ({				\
		const typeof(((type *)0)->member) *__mptr = (ptr);	\
		(type *)((char *)__mptr - offsetof(type,member));})
