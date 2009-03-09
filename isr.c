#include "port.h"
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

static void *isr_routines[256] = {0};

void register_isr(unsigned irq, void (*handler)(struct regs *r))
{
	if (isr_routines[irq])
		printf("WARNING: re-assigning IRQ %d\n", irq);
	isr_routines[irq] = handler;
}

static void irq_remap(void)
{
	outportb(0x20, 0x11);
	outportb(0xA0, 0x11);
	outportb(0x21, 0x20);
	outportb(0xA1, 0x28);
	outportb(0x21, 0x04);
	outportb(0xA1, 0x02);
	outportb(0x21, 0x01);
	outportb(0xA1, 0x01);
	outportb(0x21, 0x0);
	outportb(0xA1, 0x0);
}

void isrs_install(void)
{
	unsigned i;
	extern void int_stubs(void);
	char *p = (char *)int_stubs;

	irq_remap();
	/* Each IRQ stub is 9 bytes long, so we just allocate them in this
	 * loop. */
	for (i = 0; i < 47; i++, p += 9)
		idt_set_gate(i, (unsigned)p, 0x08, 0x8E);

	/* Enable IRQs */
	__asm__ __volatile__ ("sti");
}

static void exception_msg(unsigned ex)
{
	puts("Fatal exception: ");
	if (ex < ARRAY_SIZE(exceptions))
		puts(exceptions[ex]);
	else
		puts("Reserved Exception");
	puts("\nPanicking: System Halted!\n");
	for (;;);
}

static void irq_handler(struct regs *r)
{
	/* If the IDT entry that was invoked was greater than 40
	 * (meaning IRQ8 - 15), then we need to send an EOI to the slave
	 * controller. */
	if (r->int_no >= 40)
		outportb(0xA0, 0x20);

	/* In either case, we need to send an EOI to the master interrupt
	 * controller too. */
	outportb(0x20, 0x20);
}

void isr_handler(struct regs *r)
{
	void (*handler)(struct regs *r);
	handler = isr_routines[r->int_no];
	if (handler) {
		handler(r);
	} else {
		if (r->int_no < 32)
			/* This shouldn't return */
			exception_msg(r->int_no);
		else
			printf("Unhandled IRQ %d\n", r->int_no-32);
	}
	if (r->int_no > 31)
		irq_handler(r);
}
