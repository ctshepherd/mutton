#include "system.h"

int main(void)
{
	gdt_install();
	idt_install();
	isrs_install();
	init_video();
	puts("Hello world!\n");
	for (;;);
	return 0;
}
