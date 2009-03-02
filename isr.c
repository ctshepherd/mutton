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

static void *irq_routines[256] = {0};

void register_irq(unsigned irq, void (*handler)(struct regs *r))
{
	if (irq_routines[irq])
		printf("WARNING: re-assigning IRQ %d\n", irq);
	irq_routines[irq] = handler;
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

	/* Exceptions */
	for (i = 0; i < 31; i++, p += 9)
		idt_set_gate(i, (unsigned)p, 0x08, 0x8E);

	/* Now we do IRQs */
	irq_remap();
	for (; i < 47; i++, p += 9)
		idt_set_gate(i, (unsigned)p, 0x08, 0x8E);

	/* Enable IRQs */
	__asm__ __volatile__ ("sti");
}

static void exception_handler(struct regs *r)
{
	puts("Fatal exception: ");
	if (r->int_no < ARRAY_SIZE(exceptions))
		puts(exceptions[r->int_no]);
	else
		puts("Reserved Exception");
	puts("\nPanicking: System Halted!\n");
	for (;;);
}

static void irq_handler(struct regs *r)
{
	void (*handler)(struct regs *r);

	/* Find out if we have a custom handler to run for this IRQ, and then
	 * run it. */
	handler = irq_routines[r->int_no - 32];
	if (handler)
		handler(r);
	else
		printf("Unhandled IRQ %d\n", r->int_no - 32);

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
	if (r->int_no < 32)
		/* This shouldn't return */
		exception_handler(r);
	else
		irq_handler(r);
}
