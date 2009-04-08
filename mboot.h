#include "type.h"

/* The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

#define MBOOT_MEM_FIELDS	(1 << 0)
#define MBOOT_BOOT_DEVICE	(1 << 1)
#define MBOOT_CMDLINE		(1 << 2)
#define MBOOT_MODS		(1 << 3)
#define MBOOT_AOUT_KERNEL	(1 << 4)
#define MBOOT_ELF_KERNEL	(1 << 5)
#define MBOOT_MMAP_FIELDS	(1 << 6)
#define MBOOT_DRIVE_FIELDS	(1 << 7)
#define MBOOT_CONFIG_TABLE	(1 << 8)
#define MBOOT_BOOT_LOADER	(1 << 9)
#define MBOOT_APM_TABLE		(1 << 10)

/* The symbol table for a.out.  */
struct aout_symbol_table {
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
};

/* The section header table for ELF.  */
struct elf_section_header_table {
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
};

struct module_header_table {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
};

/* The Multiboot information.  */
struct mboot_info {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	union {
		struct aout_symbol_table aout_sym;
		struct elf_section_header_table elf_sec;
	} u;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t boot_loader_name;
	uint32_t apm_table;
} __attribute__((packed));

/* The memory map. Be careful that the offset 0 is base_addr_low
   but no size.  */
struct memory_map {
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
};

void load_mboot(struct mboot_info *m);
void print_mboot(void);
