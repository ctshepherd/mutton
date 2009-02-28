#include "screen.h"
#include "system.h"

int main(void)
{
	gdt_install();
	idt_install();
	isrs_install();
	puts("Hello world!\n");
	for (;;);
	return 0;
}
