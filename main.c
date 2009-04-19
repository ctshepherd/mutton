#include "initrd.h"
#include "malloc.h"
#include "mboot.h"
#include "screen.h"
#include "string.h"
#include "system.h"
#include "vfs.h"

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
	puts(" done!\n");

	puts("Initialising initrd fs...");
	init_initrd();
	puts(" done!\n");
}

static void time_malloc(void)
{
	unsigned start, end;
	puts("Testing malloc...\n");
	start = gettimeofday();
	test_malloc();
	end = gettimeofday();
	printf("Malloc testing complete: %u seconds elapsed, %u pages used\n", end-start, pages_allocated());
}

static void print_directory(struct vfs_inode *f, unsigned indent)
{
	unsigned i = 0;
	struct vfs_dirent *node;
	while ((node = vfs_readdir(f, i++)) != NULL) {
		struct vfs_inode *fsnode;
		puts("\nFound file \"");
		puts(node->name);
		fsnode = vfs_finddir(f, node->name);

		if (fsnode == ERROR_PTR) {
			puts("got error, skipping...\n");
			continue;
		}
		if (fsnode->mask & S_IFDIR) {
			puts("\" (directory):");
			if (strcmp(node->name, "."))
				print_directory(fsnode, indent+1);
			else
				puts(" (same file)");
		} else {
			puts("\" (regular file)");
		}
	}
}

int main(unsigned magic, struct mboot_info *m)
{
	cls();
	if (magic == 0x2BADB002)
		load_mboot(m);
	else
		puts("Not loaded by a multiboot bootloader");
	init();

	puts("Hello world!\n");
	assert(m->mods_count > 0);
	uint32_t initrd_location = *((uint32_t*)m->mods_addr);
	uint32_t initrd_end = *(uint32_t*)(m->mods_addr+4);
	struct superblock *s = mount_fs("initrd", (char *)initrd_location, initrd_end-initrd_location);
	print_directory(s->root, 0);

	for (;;);
	return 0;
}
