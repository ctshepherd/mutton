#include "string.h"
#include "system.h"

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

/* Declare an IDT of 256 entries. Unused entries exist as a trap - if any
 * undefined IDT entry is hit, it normally will cause an "Unhandled Interrupt"
 * exception: Any descriptor for which the 'presence' bit is cleared (0)
 * generates an "Unhandled Interrupt" exception */
static struct idt_entry idt[256];
static struct idt_ptr idtp = {
	.limit = sizeof(idt) - 1,
	.base = (unsigned int)&idt,
};

/* Sets an entry in the IDT. */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
		unsigned char flags)
{
	idt[num].base_lo = (base & 0xFFFF);
	idt[num].base_hi = (base >> 16) & 0xFFFF;

	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = flags;
}

void idt_install(void)
{
	memset(&idt, 0, sizeof(idt));
	/* Points the processor's internal register to the new IDT */
	asm("lidt %0\n" : :"m"(idtp));
}
