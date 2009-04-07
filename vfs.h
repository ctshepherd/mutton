#ifndef _VFS_H
#define _VFS_H

#include "type.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06

// One of these is returned by the readdir call, according to POSIX.
struct vfs_dirent {
	char name[128]; // Filename.
	uint32_t ino;     // Inode number. Required by POSIX.
};

struct filesystem;

struct vfs_inode {
	uint32_t mask;        // The permissions mask.
	uint32_t uid;         // The owning user.
	uint32_t gid;         // The owning group.
	uint32_t flags;       // Includes the node type. See #defines above.
	uint32_t ino;   // This is device-specific - provides a way for a filesystem to identify files.
	uint32_t length;      // Size of the file, in bytes.
	struct superblock *super;
};

typedef unsigned (*read_type_t)(struct vfs_inode *f, unsigned offset, unsigned size, char *buf);
typedef unsigned (*write_type_t)(struct vfs_inode *f, unsigned offset, unsigned size, char *buf);
typedef struct vfs_inode *(*open_type_t)(struct superblock *s, uint32_t inode);
typedef void (*close_type_t)(struct vfs_inode *f);
typedef struct vfs_dirent *(*readdir_type_t)(struct vfs_inode *f, unsigned index);
typedef struct vfs_inode *(*finddir_type_t)(struct vfs_inode *f, char *name);

struct filesystem_ops {
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
};

struct filesystem {
	const char *name;
	const struct filesystem_ops *ops;
};

struct superblock {
	const struct filesystem *fs;
	void *priv;
};

#define ERROR_PTR		((void *)-1)
#define GET_FS_OP(f, op)	((f)->super->fs->ops->op)

unsigned vfs_read(struct vfs_inode *node, unsigned offset, size_t size, char *buffer);
unsigned vfs_write(struct vfs_inode *node, unsigned offset, size_t size, char *buffer);
struct vfs_inode *vfs_open(char *name, unsigned flags);
void vfs_close(struct vfs_inode *node);
struct vfs_dirent *vfs_readdir(struct vfs_inode *node, unsigned index);
struct vfs_inode *vfs_finddir(struct vfs_inode *node, char *name);

unsigned vfs_register_fs(const struct filesystem *fs);

struct vfs_inode *vfs_finddir_wrapper(struct vfs_inode *f, char *name);

#endif
