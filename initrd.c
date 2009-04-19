/*
 * Initrd format:
 *  - uint32: version
 *  - uint32: checksum (xors with version to give initrd magic num)
 *  - Array of initrd_inode's, terminated by NULL entry
 *  - uint32_t: amount of initrd_disk_dir's to follow
 *  - Array of initrd_disk_dir's, that is, with num uint32_t's for each inode
 *    then num strings.
 *  - Raw data, sorted by inode
 */

#include "vfs.h"
#include "screen.h"
#include "string.h"
#include "system.h"
#include "malloc.h"

#include "initrd.h"

static const struct filesystem_ops initrd_fs_ops;
static const struct filesystem initrd_fs;

static unsigned initrd_read(struct vfs_inode *f, unsigned offset, unsigned size, char *buf)
{
	struct initrd_superblock *sb = f->super->priv;
	struct initrd_inode *i = &sb->inode_list[f->ino];
	memcpy(buf, sb->contents+(i->offset+offset), size);
	return size;
}

static unsigned initrd_open(struct superblock *s, uint32_t inode, struct vfs_inode *f)
{
	struct initrd_superblock *sb = s->priv;
	struct initrd_inode *di;

	if (inode > sb->inode_num)
		return 1;
	di = &sb->inode_list[inode];
	f->mask = di->mask;
	assert(f->mask);
	f->uid = di->uid;
	f->gid = di->gid;
	f->ino = di->ino;
	f->length = di->length;
	f->super = s;

	return 0;
}

static struct initrd_online_dir *process_disk_dirs(struct initrd_disk_dir *disk_dirs, unsigned num, char **contents)
{
	unsigned i;
	struct initrd_online_dir *online_dirs = malloc(num*sizeof(struct initrd_online_dir));
	if (!online_dirs)
		return NULL;

	for (i = 0; i < num; i++) {
		char *s;
		unsigned j;
		struct initrd_subdir *sd = malloc(disk_dirs->num*sizeof(struct initrd_subdir));
		if (!sd)
			goto err;
		online_dirs[i].ino = disk_dirs->ino;
		online_dirs[i].num = disk_dirs->num;
		online_dirs[i].subdirs = sd;
		s = ((char *)disk_dirs->subdirs)+disk_dirs->num*sizeof(uint32_t);
		for (j = 0; j < disk_dirs->num; j++) {
			sd[j].ino = disk_dirs->subdirs[j];
			sd[j].name = s;
			s += strlen(s);
			s = ALIGN(s, 4);
		}
		disk_dirs = (struct initrd_disk_dir *)s;
	}
	*contents = (char *)disk_dirs;
	return online_dirs;
err:
	for ( ; i; i--)
		free(online_dirs[i].subdirs);
	free(online_dirs);
	return ERROR_PTR;
}

static struct initrd_online_dir *initrd_find_dir_entry(struct initrd_superblock *s, uint32_t ino)
{
	unsigned mid, beg, end, i = s->dir_num;
	beg = 0;
	end = s->dir_num-1;
	while (i) {
		mid = beg+(end - beg)/2;
		if (ino < s->dir_list[mid].ino)
			end = mid;
		else if (ino > s->dir_list[mid].ino)
			beg = mid;
		else if (ino == s->dir_list[mid].ino)
			return &s->dir_list[mid];
		i /= 2;
	}
	assert(0);
	return NULL;
}

static int initrd_readdir(struct vfs_inode *f, unsigned index, struct vfs_dirent *d)
{
	struct initrd_subdir *sdir;
	struct initrd_online_dir *od;
	struct initrd_superblock *sb = (struct initrd_superblock *)f->super->priv;
	if (index == 0)
		return vfs_filldir(".", f->ino, d);
	od = initrd_find_dir_entry(sb, f->ino);
	sdir = &od->subdirs[index-1];
	if (index > od->num)
		return -1;
	return vfs_filldir(sdir->name, sdir->ino, d);
}

static struct vfs_inode *initrd_finddir(struct vfs_inode *f, char *name)
{
	return vfs_finddir_wrapper(f, name);
}

static unsigned init_initrd_fs(struct superblock *s, char *disk, size_t length)
{
	struct initrd_superblock *sb;
	struct initrd_disk_dir *d;
	struct initrd_inode *i;
	uint32_t *header;
	uint32_t version, checksum;
	char *contents = NULL;

	sb = malloc(sizeof(struct initrd_superblock));
	if (!sb)
		return 1;

	if (length < 2*4) {
		free(sb);
		return 2;
	}
	length -= (2*4);

	header = (uint32_t *)disk;
	version = *header++;
	checksum = *header++;
	if ((version^checksum) != INITRD_MAGIC) {
		free(sb);
		puts("initrd: could not load disk: invalid format\n");
		return 2;
	}
	if (version != SUPPORTED_VERSION) {
		free(sb);
		puts("initrd: could not load disk: wrong version\n");
		return 2;
	}

	i = sb->inode_list = (struct initrd_inode *)header;
	do {
		if (length < sizeof(struct initrd_inode)) {
			free(sb);
			puts("initrd: could not load disk: wrong length (inode)\n");
			return 2;
		}
		length -= sizeof(struct initrd_inode);
	} while (i->mask && i++);
	sb->inode_num = i-sb->inode_list;

	header = (uint32_t *)++i;
	sb->dir_num = *header++;
	assert(sb->dir_num);
	d = (struct initrd_disk_dir *)header;
	sb->dir_list = process_disk_dirs(d, sb->dir_num, &contents);

	if (!sb->dir_list) {
		free(sb);
		return 1;
	}
	sb->contents = contents;
	s->fs = &initrd_fs;
	s->priv = sb;
	s->root = vfs_open_ino(s, 0, RDONLY);

	return 0;
}

static const struct filesystem_ops initrd_fs_ops = {
	.read = initrd_read,
	.write = NULL,
	.open = initrd_open,
	.delinode = NULL,
	.readdir = initrd_readdir,
	.finddir = initrd_finddir,
};

static const struct filesystem initrd_fs = {
	.name = "initrd",
	.ops = &initrd_fs_ops,
	.initfs = init_initrd_fs,
};

unsigned init_initrd(void)
{
	return vfs_register_fs(&initrd_fs);
}
