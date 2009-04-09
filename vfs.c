#include "malloc.h"
#include "screen.h"
#include "string.h"
#include "system.h"
#include "vfs.h"

/* VFS stubs to keep basic parameter checking out of actual fs code */

#define VFS_STUB(op)				\
	op##_type_t op = NULL;			\
	if (f)					\
		op = GET_FS_OP(f, op);		\

unsigned vfs_init_inode(struct vfs_inode *f)
{
	memset(f, 0, sizeof(struct vfs_inode));
	return 0;
}

unsigned vfs_read(struct vfs_inode *f, unsigned offset, size_t size, char *buffer)
{
	VFS_STUB(read);
	if (!read || !buffer)
		return 0;
	if (offset+size > f->length)
		size = f->length - offset;
	return read(f, offset, size, buffer);
}

unsigned vfs_write(struct vfs_inode *f, unsigned offset, size_t size, char *buffer)
{
	VFS_STUB(write);
	if (!write || !buffer)
		return 0;
	return write(f, offset, size, buffer);
}

/* Unsupported atm */
struct vfs_inode *vfs_open_path(char *name, unsigned flags)
{
	return NULL;
}

struct vfs_inode *vfs_open_ino(struct superblock *s, unsigned inode, unsigned flags)
{
	struct vfs_inode *i;
	open_type_t open;

	if (!s)
		return ERROR_PTR;
	open = s->fs->ops->open;
	if (!open)
		return ERROR_PTR;

	i = open(s, inode);
	if (!i)
		return ERROR_PTR;

	switch (flags) {
	case RDWRITE:
	case RDONLY:
	case WRONLY:
		if (!(i->flags & flags))
			return ERROR_PTR;
		break;
	default:
		assert(0);
		return ERROR_PTR;
	}

	i->refcount++;
	return i;
}


void vfs_close(struct vfs_inode *f)
{
	if (!f)
		return;
	assert(f->refcount);
	f->refcount--;
	if (!f->refcount) {
		VFS_STUB(delinode);
		if (delinode)
			delinode(f);
	}
}

struct vfs_dirent *vfs_readdir(struct vfs_inode *f, unsigned index)
{
	VFS_STUB(readdir);
	if (!readdir)
		return ERROR_PTR;
	return readdir(f, index);
}

struct vfs_inode *vfs_finddir(struct vfs_inode *f, char *name)
{
	VFS_STUB(finddir);
	if (!finddir || !name)
		return ERROR_PTR;
	return finddir(f, name);
}


/* Called by filesystems to register themselves */

struct fs_list_entry {
	struct fs_list_entry *next;
	const struct filesystem *fs;
};

static struct fs_list_entry *filesystems = NULL;

unsigned vfs_register_fs(const struct filesystem *fs)
{
	struct fs_list_entry *e = malloc(sizeof(struct fs_list_entry));
	if (!e)
		return 1;
	printf("VFS was asked to register filesystem '%s' (%p)\n", fs->name, fs);
	e->next = filesystems;
	e->fs = fs;
	filesystems = e;
	return 0;
}



/*
 * VFS wrapper around readdir to provide a finddir alternative.
 * Returns NULL if name is not in f, if f is not a directory or if f is an
 * invalid filesystem.
 */
struct vfs_inode *vfs_finddir_wrapper(struct vfs_inode *f, char *name)
{
	unsigned index;
	open_type_t open = GET_FS_OP(f, open);
	readdir_type_t readdir = GET_FS_OP(f, readdir);
	if (!open || !readdir)
		return ERROR_PTR;
	if (!(f->flags & FS_DIRECTORY))
		return ERROR_PTR;

	for (index = 0; ; index++) {
		struct vfs_dirent *d = readdir(f, index);
		if (!d)
			break;
		if (!strcmp(d->name, name))
			return open(f->super, d->ino);
	}
	return NULL;
}
