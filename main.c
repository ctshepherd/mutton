#include "system.h"

int main(void)
{
	gdt_install();
	idt_install();
	init_video();
	puts("Hello world!");
	for (;;);
	return 0;
}
