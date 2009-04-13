#ifndef _VFS_H
#define _VFS_H

#include "type.h"

#ifndef _SYS_STAT_H
#define S_IFMT		0170000		/* bit mask for the file type bit fields */
#define S_IFSOCK	0140000		/* socket */
#define S_IFLNK		0120000		/* symbolic link */
#define S_IFREG		0100000		/* regular file */
#define S_IFBLK		0060000		/* block device */
#define S_IFDIR		0040000		/* directory */
#define S_IFCHR		0020000		/* character device */
#define S_IFIFO		0010000		/* FIFO */
#define S_ISUID		0004000		/* set UID bit */
#define S_ISGID		0002000		/* set-group-ID bit (see below) */
#define S_ISVTX		0001000		/* sticky bit (see below) */
#define S_IRWXU		00700		/* mask for file owner permissions */
#define S_IRUSR		00400		/* owner has read permission */
#define S_IWUSR		00200		/* owner has write permission */
#define S_IXUSR		00100		/* owner has execute permission */
#define S_IRWXG		00070		/* mask for group permissions */
#define S_IRGRP		00040		/* group has read permission */
#define S_IWGRP		00020		/* group has write permission */
#define S_IXGRP		00010		/* group has execute permission */
#define S_IRWXO		00007		/* mask for permissions for others (not in group) */
#define S_IROTH		00004		/* others have read permission */
#define S_IWOTH		00002		/* others have write permission */
#define S_IXOTH		00001		/* others have execute permission */
#endif

#define RDONLY		0x01
#define WRONLY		0x02
#define RDWRITE		0x03

// One of these is returned by the readdir call, according to POSIX.
struct vfs_dirent {
	char name[128]; // Filename.
	uint32_t ino;     // Inode number. Required by POSIX.
};

struct filesystem;

struct vfs_inode {
	uint32_t mask;        // The permissions mask (includes file type)
	uint32_t uid;         // The owning user.
	uint32_t gid;         // The owning group.
	uint32_t ino;         // This is device-specific - provides a way for a filesystem to identify files.
	uint32_t length;      // Size of the file, in bytes.
	unsigned refcount;
	struct superblock *super;
};

typedef unsigned (*read_type_t)(struct vfs_inode *f, unsigned offset, unsigned size, char *buf);
typedef unsigned (*write_type_t)(struct vfs_inode *f, unsigned offset, unsigned size, char *buf);
typedef unsigned (*open_type_t)(struct superblock *s, uint32_t inode, struct vfs_inode *f);
typedef void (*delinode_type_t)(struct vfs_inode *f);
typedef int (*readdir_type_t)(struct vfs_inode *f, unsigned index, struct vfs_dirent *d);
typedef struct vfs_inode *(*finddir_type_t)(struct vfs_inode *f, char *name);
typedef unsigned (*initfs_type_t)(struct superblock *s, char *disk, size_t length);

struct filesystem_ops {
	read_type_t read;
	write_type_t write;
	open_type_t open;
	delinode_type_t delinode;
	readdir_type_t readdir;
	finddir_type_t finddir;
};

struct filesystem {
	const char *name;
	const struct filesystem_ops *ops;
	initfs_type_t initfs;
};

struct superblock {
	const struct filesystem *fs;
	struct vfs_inode *root;
	void *priv;
};

#define ERROR_PTR		((void *)-1)
#define GET_FS_OP(f, op)	((f)->super->fs->ops->op)

unsigned vfs_read(struct vfs_inode *node, unsigned offset, size_t size, char *buffer);
unsigned vfs_write(struct vfs_inode *node, unsigned offset, size_t size, char *buffer);
struct vfs_inode *vfs_open_path(char *name, unsigned flags);
struct vfs_inode *vfs_open_ino(struct superblock *s, unsigned inode, unsigned flags);
void vfs_close(struct vfs_inode *node);
struct vfs_dirent *vfs_readdir(struct vfs_inode *node, unsigned index);
struct vfs_inode *vfs_finddir(struct vfs_inode *node, char *name);

unsigned vfs_register_fs(const struct filesystem *fs);
unsigned vfs_init_inode(struct vfs_inode *f);
struct superblock *mount_fs(char *name, char *disk, size_t length);

struct vfs_inode *vfs_finddir_wrapper(struct vfs_inode *f, char *name);
unsigned vfs_filldir(const char *name, uint32_t ino, struct vfs_dirent *d);

#endif
