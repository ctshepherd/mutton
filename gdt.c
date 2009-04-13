#include "system.h"

struct gdt_entry {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

static void gdt_flush(void)
{
	asm("lgdt %0\n\t"
		"movw $0x10,%%ax\n\t" /* 0x10 is the offset in the GDT to our data segment */
		"movw %%ax,%%ds\n\t"
		"movw %%ax,%%es\n\t"
		"movw %%ax,%%fs\n\t"
		"movw %%ax,%%gs\n\t"
		"movw %%ax,%%ss\n\t"
		"ljmp $0x8, $flush2\n\t" /* 0x08 is the offset to our code segment: Far jump! */
		"flush2:\n\t"
		: :"m"(gp));
}


/* Setup a descriptor in the Global Descriptor Table */
static void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	/* Setup the descriptor base address */
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

void gdt_install(void)
{
	/* Setup the GDT pointer and limit */
	gp.limit = sizeof(gdt) - 1;
	gp.base = (unsigned int)&gdt;

	/* Our NULL descriptor */
	gdt_set_gate(0, 0, 0, 0, 0);
	/* Code Segment */
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	/* Data Segment */
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	/* Flush out the old GDT and install the new one */
	gdt_flush();
}
