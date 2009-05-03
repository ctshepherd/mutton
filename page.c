#include "page.h"
#include "screen.h"
#include "string.h"
#include "system.h"

static struct pte *ident_page_table;

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
	struct pte *page_directory, *stack_page_table;
	unsigned long address;
	unsigned i;

	/*
	 * Set up the start of the base page allocator.
	 * We start allocating pages from the page that begins at the end of
	 * the kernel.
	 * Note: it will not work before init_paging is called!
	 */
	alloc_page_cur_addr = frame_addr(page_align(&kernel_end));

	page_directory = alloc_page();
	ident_page_table = alloc_page();
	stack_page_table = alloc_page();

	/* Map the first 4MB of memory */
	for (address = 0, i = 0; i < 1024; i++, address += 4096) {
		set_page_attr(ident_page_table[i], address, PAGE_SUPERVISOR,
				PAGE_RW, 1);
	}

	memset(stack_page_table, 0, 4096);
	/* Map the two pages, starting at sys_stack_end */
	/* We make sure sys_stack_end is page aligned in start.asm */
	set_page_attr(stack_page_table[0], &sys_stack_end, PAGE_SUPERVISOR,
			PAGE_RW, 1);
	set_page_attr(stack_page_table[1], &sys_stack_end+0x1000, PAGE_SUPERVISOR,
			PAGE_RW, 1);

	memset(page_directory, 0, 4096);
	/* Identity map the first 4MB */
	set_page_attr(page_directory[0], ident_page_table, PAGE_SUPERVISOR,
			PAGE_RW, 1);

	/* Map the processes stack at 0x80000 */
	set_page_attr(page_directory[2], stack_page_table, PAGE_SUPERVISOR,
			PAGE_RW, 1);

	register_isr(14, page_fault_handler);
	load_page_dir(page_directory);
	enable_paging();
}
