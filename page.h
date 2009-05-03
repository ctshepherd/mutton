#include "type.h"

/*
 * 31..............12	11...9	8...7		6	5	4...3		2	1	0
 * Page frame address	Avail	Reserved	D	A	Reserved	U/S	R/W	Present
 *
 * Page frame address = Physical address of memory (either the physical address
 * 		of the page, or the physical address of the page table)
 * Avail = Do what you want with this
 * Reserved = Reserved by Intel
 * D = Dirty
 * A = Accessed
 * U/S = User or supervisor level
 * R/W = Read or read and write
 */

struct pte {
	unsigned char present:1;
	unsigned char rw:1;
	unsigned char level:1;
	unsigned char access:1;
	unsigned char dirty:1;
	unsigned reserved1:2;
	unsigned avail:3;
	unsigned reserved2:2;
	unsigned frame_addr:20;
} __attribute__((packed));

#define frame_addr(a)		(((unsigned long)(a)) >> 12)
#define phys_addr(a)		((void *)((a) << 12))
#define page_align(a)		ALIGN(a, 0x1000)

#define set_page_attr(pg, a, l, r, p) do {			\
	pg.frame_addr = frame_addr(a);				\
	pg.level = l;						\
	pg.rw = r;						\
	pg.present = p;						\
} while (0)

/* Levels */
#define PAGE_SUPERVISOR		0
#define PAGE_USER		1
/* Read/Write */
#define PAGE_RONLY		0
#define PAGE_RW			1

static inline void enable_paging(void)
{
	unsigned dummy;
	asm volatile("movl %%cr0, %0\n\t"
			"orl $0x80000000, %0\n\t"
			"movl %0, %%cr0\n"
			: "=&r"(dummy): );
}

static inline void disable_paging(void)
{
	unsigned dummy;
	asm volatile("movl %%cr0, %0\n\t"
			"andl $0x7FFFFFFF, %0\n\t"
			"movl %0, %%cr0\n"
			: "=&r"(dummy): );
}

static inline void load_page_dir(struct pte *page_directory)
{
	asm volatile("mov %0, %%cr3": : "r"(page_directory));
}

static inline struct pte *get_cur_page_dir(void)
{
	struct pte *page_dir;
	asm volatile("mov %%cr3, %0": "=r"(page_dir));
	return page_dir;
}

extern unsigned kernel_end;
extern unsigned sys_stack_end;

void *alloc_page(void);
void free_page(void *addr);
unsigned pages_allocated(void);
void init_paging(void);
