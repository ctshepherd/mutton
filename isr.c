#include "screen.h"
#include "system.h"

static const char *exceptions[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check"
};

void isrs_install(void)
{
	unsigned i;
	extern char *int_stubs;
	char *p = int_stubs;
	for (i = 0; i < 31; i++, p += 9)
		idt_set_gate(i, (unsigned)p, 0x08, 0x8E);
}

void fault_handler(struct regs *r)
{
	puts("Fatal exception: ");
	if (r->int_no < 32) {
		if (r->int_no < ARRAY_SIZE(exceptions))
			puts(exceptions[r->int_no]);
		else
			puts("Reserved Exception");
		puts("\nPanicking: System Halted!\n");
		for (;;);
	}
}
