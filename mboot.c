#include "screen.h"
#include "mboot.h"

static struct mboot_info mboot;
static void *mboot_addr;

void load_mboot(struct mboot_info *mb)
{
	mboot_addr = mb;
	mboot = *mb;
}

void print_mboot(void)
{
	printf("Multiboot flags: %X (header at %p)\n", mboot.flags, mboot_addr);

	if (mboot.flags & MBOOT_CMDLINE)
		printf("cmdline: %s @ %p\n", (char *)mboot.cmdline, (void *)mboot.cmdline);
	else
		puts("No cmdline given\n");

//	if (mboot.flags & MBOOT_MODS)
		printf("%u mods loaded at %p\n", mboot.mods_count, (void *)mboot.mods_addr);
//	else
//		puts("No modules loaded\n");

	if (mboot.flags & MBOOT_AOUT_KERNEL) {
		if (mboot.flags & MBOOT_ELF_KERNEL)
			puts("Something went horribly wrong fool\n");
		else
			puts("a.out format kernel\n");
	} else if (mboot.flags & MBOOT_ELF_KERNEL) {
		puts("ELF format kernel\n");
	} else{
		puts("Unknown type of kernel\n");
	}

	if (mboot.flags & MBOOT_BOOT_LOADER)
		printf("Bootloader: %s @ %p\n", (char *)mboot.boot_loader_name, (void *)mboot.boot_loader_name);
	else
		puts("No boot loader name\n");
}
