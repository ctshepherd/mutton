#include "bochs.h"
#include "screen.h"
#include "string.h"
#include "system.h"
#include "type.h"

/* A section is defined as a header, followed by size bytes, followed by a
 * closing header. */

/* size is the header between two headers */
struct header {
	unsigned size;
	unsigned chksum;
};

#define section_header(section)	((section)->size ^ (section)->chksum)

#define FREE_TYPE_HEADER	0xF2EE0000
#define ALLOC_TYPE_HEADER	0xA11C0000
#define TYPE_MASK		0xFFFF0000
#define section_type(s) ((s) & TYPE_MASK)

#define START_POS_HEADER	0x5727
#define END_POS_HEADER		0x0E4D
#define POS_MASK		0xFFFF
#define header_pos(s) ((s) & POS_MASK)
/* Return a pointer to the section area */
static void *section_area(struct header *section)
{
	char *p = (char *)section;
	p += sizeof(struct header);
	return (void *)p;
}

/* Return a pointer to the next section */
static struct header *end_section(struct header *section)
{
	char *p = (char *)section;
	p += section->size + sizeof(struct header);
	return (struct header *)p;
}

static unsigned valid_section(struct header *a)
{
	struct header *b;
	unsigned a_type, b_type;

	bochs_break();
	a_type = section_header(a) & TYPE_MASK;
	if ((a_type != FREE_TYPE_HEADER && a_type != ALLOC_TYPE_HEADER) ||
		(section_header(a) & POS_MASK) != START_POS_HEADER)
		return 0;

	b = end_section(a);
	b_type = section_header(b) & TYPE_MASK;
	if ((b_type != FREE_TYPE_HEADER && b_type != ALLOC_TYPE_HEADER) ||
		b_type != a_type ||
		(section_header(b) & POS_MASK) != END_POS_HEADER)
		return 0;

	if (a->size != b->size)
		return 0;

	return 1;
}

static void print_header(struct header *h)
{
	printf("Header at %p ", h);

	switch (section_header(h) & TYPE_MASK) {
	case FREE_TYPE_HEADER:
		printf("free section, ");
		break;
	case ALLOC_TYPE_HEADER:
		printf("alloced section, ");
		break;
	default:
		printf("unknown type header %X, ", section_header(h) & TYPE_MASK);
	}

	switch (section_header(h) & POS_MASK) {
	case START_POS_HEADER:
		printf("start header, ");
		break;
	case END_POS_HEADER:
		printf("end header, ");
		break;
	default:
		printf("unknown pos header %X, ", section_header(h) & POS_MASK);
	}

	printf("of size %u\n", h->size);
}

static void print_section(struct header *a)
{
	print_header(a);
	print_header(end_section(a));
}

struct section_list {
	struct header *area;
	unsigned size;
};

/* There's no point storing sections smaller than this, they're just a waste of memory. */
#define MIN_SECTION_SIZE	64
/* The number of pages free_list will take up */
#define FREE_LIST_PAGES		3
#define MAX_FREE_ELEMS		((4096/sizeof(struct section_list))*FREE_LIST_PAGES)
/* free_list is a sorted array, smallest to biggest. */
static struct section_list free_list[MAX_FREE_ELEMS];
static unsigned elem_num = 0;

/* Binary search the free_list array for the smallest element bigger than n. */
static int find_area(unsigned n)
{
	unsigned beg, end, mid, i = elem_num;

	beg = 0;
	end = elem_num-1;

	while (i) {
		mid = beg+(end - beg)/2;
		if (n < free_list[mid].size)
			end = mid;
		else if (n > free_list[mid].size)
			beg = mid;
		else if (n == free_list[mid].size)
			return mid;
		i /= 2;
	}

	/* We didn't get an exact match, so do the best we can */
	if (free_list[beg+1].size > n) {
		if (free_list[beg].size < n) {
			/* Pick the next highest option if there is one */
			return beg+1;
		} else {
			/* We must be at the start of the list */
			assert(beg == 0);
			return 0;
		}
	} else {
		/* Every item in the list is smaller than what we've been
		 * asked for */
		return -1;
	}
}

/* Get an appropriately sized area or create one */
static int get_area(unsigned n)
{
	//printf("get_area called for size %u, ", n);
	int i = find_area(n);

	if (i >= 0) {
		//printf("returning area %d\n", i);
		return i;
	}

	/* There's no available section in the free_list - create our own. */

	//printf("no free area\n");
	if (elem_num == MAX_FREE_ELEMS) {
		//printf("Can't handle allocation: %u items in free_list\n", elem_num);
		/*
		 * If our free list is full, just fail the allocation - this
		 * is really unlikely, unless we have a lot of fragmentation.
		 */
		return -1;
	}

	/* Append a new page to our free list. We put it to the end as
	 * the max size will be a page anyway. */
	struct section_list *new = &free_list[elem_num++];
	new->area = alloc_page();
	new->size = 4096-(sizeof(struct header)*2);
	struct header *a = new->area, *b;
	a->size = new->size;
	a->chksum = new->size ^ (FREE_TYPE_HEADER | START_POS_HEADER);
	b = end_section(a);
	b->size = new->size;
	b->chksum = new->size ^ (FREE_TYPE_HEADER | END_POS_HEADER);
	printf("Put new item at %u in free_list (%p), with area %p (size %u %u %u)\n",
			elem_num-1, new, new->area, new->size, a->size, b->size);
	return elem_num-1;
}


void *malloc(size_t n)
{
	/* Can't handle requests bigger than a page */
	if (n > 4096)
		return NULL;
	int i = get_area(n);
	if (i < 0)
		return NULL;

	/* Resize the section */
	struct header *a = free_list[i].area, *b = end_section(a);
	int size = free_list[i].size;
	int new_size = size - n - sizeof(struct header)*2;
	//printf("Got area %p (end section %p) of size %d (for allocation of size %d). "
	//		"The new size is %u (struct header is %u bytes big).\n",
	//		a, b, size, n, new_size, sizeof(struct header));
	if (new_size < MIN_SECTION_SIZE) {
		/* The area is either perfectly sized or (more likely) the
		 * remainder is too small to be useful. Pop it off the list
		 * and return it. */
		/* | list | old elem | <- | rest of list | */
		memmove(&free_list[i], &free_list[i+1], (elem_num-i)*sizeof(struct section_list));
		elem_num--;
		//printf("Moving %p to %p (%u bytes), with %u in free_list now\n",
		//		&free_list[i], &free_list[i+1],
		//		(elem_num-i)*sizeof(struct section_list),
		//		elem_num);

		/* Change the area headers to reflect its new allocated nature */
		a->chksum = (ALLOC_TYPE_HEADER | START_POS_HEADER)^size;
		b->chksum = (ALLOC_TYPE_HEADER |   END_POS_HEADER)^size;
		return section_area(a);
	}

	/* Split the physical headers into two. */
	struct header *new_a, *new_b = b;
	size = n;

	a->size = size;
	a->chksum = (ALLOC_TYPE_HEADER | START_POS_HEADER)^size;
	b = end_section(a);
	b->size = size;
	b->chksum = (ALLOC_TYPE_HEADER |   END_POS_HEADER)^size;

	new_a = b+1;
	new_a->size = new_size;
	new_a->chksum = (FREE_TYPE_HEADER | START_POS_HEADER)^new_size;
	new_b->size = new_size;
	new_b->chksum = (FREE_TYPE_HEADER |   END_POS_HEADER)^new_size;
	//printf("Split area %p into two: starting at %p and %p (ends: %p and %p). They are size %u and %u\n",
	//		a, a, new_a, b, new_b, size, new_size);

	/* We find the index of the next free area bigger than new_size. This
	 * needs to move down to the index of the old area */
	int new_index = find_area(new_size);
	/* | list | <new elem> | more of list | -> | old elem | */
	if (new_index == -1) {
		new_index = elem_num;
		//printf("All elements smaller, appending new entry to the list in position %d\n", new_index);
	} else {
		memmove(&free_list[new_index+1], &free_list[new_index], (i-new_index)*sizeof(struct section_list));
		//printf("Moving %p to %p (%u bytes), with %u in free_list now (new index is %d)\n",
		//		&free_list[new_index+1],
		//		&free_list[new_index],
		//		i-new_index*sizeof(struct section_list),
		//		elem_num, new_index);
	}
	free_list[new_index].area = new_a;
	free_list[new_index].size = new_size;

	return section_area(a);
}


void free(void *p)
{
	struct header *a = p, *b;

	/* Sanity checks */
	if (!valid_section(a)) {
		printf("WARNING: free was handed invalid section %p!!!\n", p);
		return;
	}
	if ((section_header(a) & TYPE_MASK) != ALLOC_TYPE_HEADER) {
		printf("WARNING: free was handed already free section %p!!!\n", p);
		return;
	}

	if (elem_num == MAX_FREE_ELEMS) {
		/* Uh-oh. Our free list is full. We'll have to discard this
		 * section - ie, leak it. XXX: This is unsatisfactory in the
		 * long term. */
		printf("WARNING: free_list is full with %u elements: cannot add any more!!! Leaking section %p!!!\n",
				elem_num, p);
		return;
	}

	int index = find_area(a->size);
	/* Insert the entry in the free_list */
	/* | list | <new elem> | more of list -> | */
	memmove(&free_list[index+1], &free_list[index], elem_num-index*sizeof(struct section_list));
	free_list[index].area = a;
	free_list[index].size = a->size;

	a->chksum = (FREE_TYPE_HEADER | START_POS_HEADER)^a->size;
	b = end_section(a);
	b->chksum = (FREE_TYPE_HEADER |   END_POS_HEADER)^a->size;
	elem_num++;
}

void *realloc(void *p, size_t n)
{
	struct header *a = p;

	if (!p)
		return malloc(n);

	/* Sanity checks */
	if (!valid_section(a)) {
		printf("WARNING: realloc was handed invalid section %p!!!\n", p);
		return NULL;
	}
	if ((section_header(a) & TYPE_MASK) != ALLOC_TYPE_HEADER) {
		printf("WARNING: realloc was handed already free section %p!!!\n", p);
		return NULL;
	}

	if (n < a->size)
		return p;

	/* Simple solution.
	 * XXX: Make this cleverer - adjacent free sections etc */
	free(p);
	return malloc(n);
}
