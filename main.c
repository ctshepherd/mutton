#include "bochs.h"
#include "malloc.h"
#include "screen.h"
#include "system.h"

void panic(const char *msg)
{
	printf("PANIC! Reason: %s\n", msg);
	stack_unwind();
	while (1); /* Spin */
}

void _assert(const char *statement, const char *func, unsigned line)
{
	printf("Assertion '%s' in function %s (line %u) failed\n",
			statement, func, line);
	panic("Assertion failed!\n");
}

void stack_unwind(void)
{
	void **frame;
	printf("Call stack:");
	for (frame = __builtin_frame_address(0);
		frame != NULL && frame[0] != NULL;
		frame = frame[0]) {
		printf(" %p", frame[1]);
	}
	printf(".\n");
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

	puts("Initialising paging...");
	init_paging();
	puts(" done!\nInitialising PIT...");
	init_pit();
	puts(" done!\nInitialising keyboard...");
	init_kbd();

	puts("done!\n");
}

int main(void)
{
	unsigned start, end;
	cls();
	init();
	puts("Hello world!\n");
	puts("Testing malloc...\n");
	start = gettimeofday();
	test_malloc();
	end = gettimeofday();
	printf("Malloc testing complete: %u seconds elapsed, %u pages used\n", end-start, pages_allocated());
	for (;;);
	return 0;
}
