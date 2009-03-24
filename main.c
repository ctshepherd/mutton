#include "screen.h"
#include "system.h"

void panic(const char *msg)
{
	printf("PANIC! Reason: %s\n", msg);
	while (1); /* Spin */
}

/* Init function: load anything and everything needed. */
static void init(void)
{
	/* Load up real basic stuff */
	puts("Installing gdt...");
	gdt_install();
	puts(" done!\nInstalling idt...");
	idt_install();
	puts(" done!\nInstalling isrs...");
	isrs_install();
	puts(" done!\n");

	puts("Initialising PIT...");
	init_pit();
	puts(" done!\nInitialising keyboard...");
	init_kbd();

	puts("done!\n");
}

int main(void)
{
	cls();
	init();
	puts("Hello world!\n");
	for (;;);
	return 0;
}
