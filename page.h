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

#define frame_addr(a)		(((unsigned long)(a))>>12)
#define page_align(a)		ALIGN(a, 0x1000)

/* Levels */
#define PAGE_SUPERVISOR		0
#define PAGE_USER		1
/* Read/Write */
#define PAGE_RONLY		0
#define PAGE_RW			1

extern unsigned end;
#define kernel_end (&end)
void *alloc_page(void);
void free_page(void *addr);
unsigned pages_allocated(void);
void init_paging(void);
