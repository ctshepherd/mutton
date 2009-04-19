#include "page.h"
#include "screen.h"
#include "system.h"

static struct pte *page_directory;
static struct pte *page_table;

static void load_page_bits(void)
{
	unsigned dummy;
	asm volatile("mov %0, %%cr3": : "r"(page_directory));
	/* Set the paging bit in CR0 */
	asm volatile("movl %%cr0, %0\n\t"
			"orl $0x80000000, %0\n\t"
			"movl %0, %%cr0\n"
			: "=&r"(dummy): );
}

static void page_fault_handler(struct regs *r)
{
	unsigned faulting_address, present, rw, us, reserved, id;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	present = !(r->err_code & 0x1);
	rw = r->err_code & 0x2;
	us = r->err_code & 0x4;
	reserved = r->err_code & 0x8;
	id = r->err_code & 0x10;

	// Output an error message.
	printf("Page fault! ( %s%s%s%s ) at %p (EIP: %p)\n",
		present ? "present " : "",
		rw ? "read-only " : "",
		us ? "user-mode" : "",
		reserved ? "reserved " : "",
		(void *)faulting_address,
		(void *)r->eip);
	panic("Page fault");
}

static unsigned long alloc_page_cur_addr;
static void *_alloc_page(void)
{
	return (void *)(alloc_page_cur_addr++ << 12);
}

/* Simple page allocator: keeps a stack of free pages, if this is empty, grows
 * the region of free pages and returns the latest addition.
 * Note: stack points to the next entry on the stack, NOT a current entry. */
static unsigned long *stack;
static unsigned stack_depth = 0;
static unsigned long alloced_pages = 0;
void *alloc_page(void)
{
	if (!stack)
		stack = _alloc_page();
	if (!stack_depth) {
		alloced_pages++;
		return _alloc_page();
	}
	assert(stack_depth);
	stack_depth--;
	return (void *)(*(--stack));
}

void free_page(void *addr)
{
	unsigned long a = (unsigned long)addr;
	*stack++ = a;
	/* Make sure we don't decrement alloced_pages when it's 0, which would
	 * wrap around */
	assert(alloced_pages);
	alloced_pages--;
	stack_depth++;
}

unsigned pages_allocated(void)
{
	return alloced_pages;
}


void init_paging(void)
{
	unsigned long address;
	unsigned i;

	/*
	 * Set up the start of the base page allocator.
	 * Note: it will not work before init_paging is called!
	 */
	alloc_page_cur_addr = frame_addr(page_align(kernel_end));

	page_directory = _alloc_page();
	page_table = _alloc_page();

	/* Map the first 4MB of memory */
	for (address = 0, i = 0; i < 1024; i++, address += 4096) {
		page_table[i].frame_addr = frame_addr(address);
		page_table[i].level = PAGE_SUPERVISOR;
		page_table[i].rw = PAGE_RW;
		page_table[i].present = 1;
	}

	/* Fill the first entry of the page directory */
	page_directory[0].frame_addr = frame_addr(page_table);
	page_directory[0].level = PAGE_SUPERVISOR;
	page_directory[0].rw = PAGE_RW;
	page_directory[0].present = 1;

	for (i = 1; i < 1024; i++) {
		page_directory[i].level = PAGE_SUPERVISOR;
		page_directory[i].rw = PAGE_RW;
		page_directory[i].present = 0;
	}
	register_isr(14, page_fault_handler);
	load_page_bits();
}
