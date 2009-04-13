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
	if (!size)
		return 0;
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
	unsigned ret;
	struct vfs_inode *i;
	open_type_t open;

	if (!s)
		return ERROR_PTR;
	open = s->fs->ops->open;
	if (!open)
		return ERROR_PTR;

	i = malloc(sizeof(struct vfs_inode));
	if (!i)
		return ERROR_PTR;
	ret = open(s, inode, i);
	if (ret)
		goto err;

	/* Assume caller is owner for the moment */
	switch (flags) {
	case RDWRITE:
		flags = S_IRUSR|S_IWUSR;
		break;
	case RDONLY:
		flags = S_IRUSR;
		break;
	case WRONLY:
		flags = S_IWUSR;
		break;
	default:
		assert(0);
		goto err;
	}

	if (!(i->mask & flags))
		goto err;

	i->refcount++;
	return i;
err:
	free(i);
	return ERROR_PTR;
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
	unsigned ret;
	struct vfs_dirent *d;
	VFS_STUB(readdir);
	if (!readdir)
		return ERROR_PTR;
	if (!(f->mask & S_IFDIR))
		return ERROR_PTR;
	d = malloc(sizeof(struct vfs_dirent));
	if (!d)
		return ERROR_PTR;
	ret = readdir(f, index, d);
	if (ret) {
		free(d);
		return ERROR_PTR;
	}
	return d;
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

struct superblock *mount_fs(char *name, char *disk, size_t length)
{
	struct superblock *s;
	struct fs_list_entry *e = filesystems;

	while (e) {
		if (!strcmp(e->fs->name, name))
			goto out;
	}
	printf("VFS could not load filesystem %s: not found\n", name);
	return NULL;

out:
	s = malloc(sizeof(struct superblock));
	if (!s)
		return NULL;
	memset(s, 0, sizeof(struct superblock));
	s->fs = e->fs;

	unsigned ret;
	if ((ret = e->fs->initfs(s, disk, length))) {
		free(s);
		printf("VFS could not load filesystem %s: got ret %u from initfs\n", name, ret);
		return NULL;
	}
	return s;
}


/*
 * VFS wrapper around readdir to provide a finddir alternative.
 * Returns NULL if name is not in f, if f is not a directory or if f is an
 * invalid filesystem.
 */
struct vfs_inode *vfs_finddir_wrapper(struct vfs_inode *f, char *name)
{
	unsigned index;
	if (!(f->mask & S_IFDIR))
		return ERROR_PTR;

	for (index = 0; ; index++) {
		struct vfs_dirent *d = vfs_readdir(f, index);
		if (!d)
			break;
		if (!strcmp(d->name, name))
			return vfs_open_ino(f->super, d->ino, RDONLY);
	}
	return NULL;
}

unsigned vfs_filldir(const char *name, uint32_t ino, struct vfs_dirent *d)
{
	if (strlen(name) > 128)
		return 1;
	strncpy(d->name, name, 127);
	d->name[127] = '\0';
	d->ino = ino;
	return 0;
}
