#include "pit.h"
#include "screen.h"
#include "system.h"

/* Init function: load anything and everything needed. */
static void init(void)
{
	/* Load up real basic stuff */
	gdt_install();
	idt_install();
	isrs_install();

	init_pit();
	init_kbd();
}

int main(void)
{
	init();
	cls();
	puts("Hello world!\n");
	for (;;);
	return 0;
}
