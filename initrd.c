/*
 * Initrd format:
 *  - Inode table
 *  - Inodes
 */

#include "vfs.h"
#include "string.h"
#include "system.h"
#include "malloc.h"

#include "initrd.h"

static const struct filesystem_ops initrd_fs_ops;
static const struct filesystem initrd_fs;

unsigned initrd_read(struct vfs_inode *f, unsigned offset, unsigned size, char *buf)
{
	struct initrd_superblock *sb = f->super->priv;
	struct initrd_inode *i = &sb->inode_list[f->ino];
	memcpy(buf, sb->contents+(i->offset+offset), size);
	return size;
}

struct vfs_inode *initrd_open(struct superblock *s, uint32_t inode)
{
	struct vfs_inode *vi = malloc(sizeof(struct vfs_inode));
	struct initrd_superblock *sb = s->priv;
	struct initrd_inode *di = &sb->inode_list[inode];

	if (!vi)
		return NULL;
	vi->mask = di->mask;
	vi->mask = di->mask;
	vi->uid = di->uid;
	vi->gid = di->gid;
	vi->flags = di->flags;
	vi->ino = di->ino;
	vi->length = di->length;
	vi->super = s;

	return vi;
}

#if 0
struct vfs_dirent *readdir(struct vfs_inode *f, unsigned index)
{
	return;
}
#endif

struct vfs_inode *initrd_finddir(struct vfs_inode *f, char *name)
{
	return vfs_finddir(f, name);
}

unsigned init_initrd_fs(struct superblock *s, char *disk, size_t length)
{
	struct initrd_superblock *sb;
	struct vfs_dirent *d;
	struct initrd_inode *i;
	uint32_t version, checksum;

	s->fs = &initrd_fs;

	sb = malloc(sizeof(struct initrd_superblock));
	if (!sb)
		return 1;

	if (length < 2*4+sizeof(struct vfs_dirent)) {
		free(sb);
		return 2;
	}
	length -= 2*4+sizeof(struct vfs_dirent);

	version = *(uint32_t *)disk++;
	checksum = *(uint32_t *)disk++;
	if ((version^checksum) != INITRD_MAGIC) {
		free(sb);
		return 2;
	}
	if (version != SUPPORTED_VERSION) {
		free(sb);
		return 2;
	}

	d = sb->file_list = (struct vfs_dirent *)disk;
	do {
		if (length < sizeof(struct vfs_dirent)) {
			free(sb);
			return 2;
		}
		length -= sizeof(struct vfs_dirent);
	} while (d->name && d++);

	i = sb->inode_list = (struct initrd_inode *)d;
	do {
		if (length < sizeof(struct initrd_inode)) {
			free(sb);
			return 2;
		}
		length -= sizeof(struct initrd_inode);
	} while (i->flags && i++);

	sb->contents = malloc(length);
	if (!sb->contents) {
		free(sb);
		return 1;
	}
	memcpy(sb->contents, (char *)i, length);

	return 0;
}

static const struct filesystem_ops initrd_fs_ops = {
	.read = initrd_read,
	.write = NULL,
	.open = initrd_open,
	.delinode = NULL,
//	.readdir = initrd_readdir,
	.readdir = NULL,
	.finddir = initrd_finddir,
};

static const struct filesystem initrd_fs = {
	.name = "Initrd",
	.ops = &initrd_fs_ops,
	.initfs = init_initrd_fs,
};

unsigned init_initrd(void)
{
	return vfs_register_fs(&initrd_fs);
}
