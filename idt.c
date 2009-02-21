#include "system.h"

/* Defines an IDT entry */
struct idt_entry {
	unsigned short base_lo;
	unsigned short sel;        /* Our kernel segment goes here! */
	unsigned char always0;     /* This will ALWAYS be set to 0! */
	unsigned char flags;
	unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
static struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_load(void);
#if 0
static void idt_load(void)
{
	asm("lidt %0\n" : :"m"(idtp));
}
#endif

/* Use this function to set an entry in the IDT. */
static void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
	idt[num].base_lo = (base & 0xFFFF);
	idt[num].base_hi = (base >> 16) & 0xFFFF;

	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = flags;
}

/* Installs the IDT */
void idt_install(void)
{
	/* Sets the special IDT pointer up, just like in 'gdt.c' */
	idtp.limit = sizeof(idt) - 1;
	idtp.base = (unsigned int)&idt;

	/* Clear out the entire IDT, initializing it to zeros */
	memset(&idt, 0, sizeof(idt));

	/* XXX: Add ISRs to the IDT */

	/* Points the processor's internal register to the new IDT */
	idt_load();
}
