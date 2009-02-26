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

void isr0(void);
void isr1(void);
void isr2(void);
void isr3(void);
void isr4(void);
void isr5(void);
void isr6(void);
void isr7(void);
void isr8(void);
void isr9(void);
void isr10(void);
void isr11(void);
void isr12(void);
void isr13(void);
void isr14(void);
void isr15(void);
void isr16(void);
void isr17(void);
void isr18(void);
void isr19(void);
void isr20(void);
void isr21(void);
void isr22(void);
void isr23(void);
void isr24(void);
void isr25(void);
void isr26(void);
void isr27(void);
void isr28(void);
void isr29(void);
void isr30(void);
void isr31(void);

#define SET_GATE(n)	idt_set_gate(n, (unsigned)isr##n, 0x08, 0x8E)

void isrs_install(void)
{
	SET_GATE(0);
	SET_GATE(1);
	SET_GATE(2);
	SET_GATE(3);
	SET_GATE(4);
	SET_GATE(5);
	SET_GATE(6);
	SET_GATE(7);
	SET_GATE(8);
	SET_GATE(9);
	SET_GATE(10);
	SET_GATE(11);
	SET_GATE(12);
	SET_GATE(13);
	SET_GATE(14);
	SET_GATE(15);
	SET_GATE(16);
	SET_GATE(17);
	SET_GATE(18);
	SET_GATE(19);
	SET_GATE(20);
	SET_GATE(21);
	SET_GATE(22);
	SET_GATE(23);
	SET_GATE(24);
	SET_GATE(25);
	SET_GATE(26);
	SET_GATE(27);
	SET_GATE(28);
	SET_GATE(29);
	SET_GATE(30);
	SET_GATE(31);
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
