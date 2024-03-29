#include "vfs.h"

#define INITRD_MAGIC (0xF154F00D ^ ~0)
#define SUPPORTED_VERSION (0x0003)

struct initrd_inode {
	uint32_t mask;        // The permissions mask.
	uint32_t uid;         // The owning user.
	uint32_t gid;         // The owning group.
	uint32_t ino;         // This is device-specific - provides a way for a filesystem to identify files.
	uint32_t length;      // Size of the file, in bytes.
	uint32_t offset;
};

struct initrd_disk_dir {
	uint32_t ino;
	uint32_t num;
	uint32_t subdirs[0];
};

struct initrd_subdir {
	const char *name;
	uint32_t ino;
};

struct initrd_online_dir {
	uint32_t ino;
	uint32_t num;
	struct initrd_subdir *subdirs;
};

struct initrd_superblock {
	struct superblock s;
	struct vfs_dirent *file_list;
	struct initrd_inode *inode_list;
	unsigned inode_num;
	struct initrd_online_dir *dir_list;
	unsigned dir_num;
	char *contents;
};

unsigned init_initrd(void);
