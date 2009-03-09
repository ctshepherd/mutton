#include "screen.h"
#include "system.h"

void panic(const char *msg)
{
	printf("PANIC! Reason: %s\n");
	while (1); /* Spin */
}

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
