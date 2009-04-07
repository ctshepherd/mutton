#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vfs.h"
#include "initrd.h"

int main(int argc, char **argv)
{
	int i;
	unsigned offset;
	FILE *w = fopen("initrd.img", "w");
	if (!w) {
		perror("initrd.img");
		return -1;
	}

	uint32_t version = SUPPORTED_VERSION, chksum = SUPPORTED_VERSION^INITRD_MAGIC;
	fwrite(&version, sizeof(uint32_t), 1, w);
	fwrite(&chksum,  sizeof(uint32_t), 1, w);

	for (i = 1; i < argc; i++) {
		struct vfs_dirent d;
		strncpy(d.name, argv[i], sizeof(d.name));
		d.ino = i-1;
		fwrite(&d, sizeof(struct vfs_dirent), 1, w);
	}
	/* Write a NULL entry */
	struct vfs_dirent d;
	memset(d.name, 0, sizeof(d.name));
	d.ino = 0;
	fwrite(&d, sizeof(struct vfs_dirent), 1, w);

	offset = argc*(sizeof(struct vfs_dirent)+sizeof(struct initrd_inode));
	for (i = 1; i < argc; i++) {
		struct initrd_inode inode;
		struct stat buf;
		if (stat(argv[i], &buf) == -1) {
			perror(argv[i]);
			return -1;
		}
		inode.mask = buf.st_mode;
		inode.uid = buf.st_uid;
		inode.gid = buf.st_gid;
		inode.flags = FS_FILE;
		inode.ino = i-1;
		inode.length = buf.st_size;
		inode.offset = offset;
		offset += buf.st_size;
	}
	struct initrd_inode inode;
	memset(&inode, 0, sizeof(struct initrd_inode));
	fwrite(&inode, sizeof(struct initrd_inode), 1, w);

	for (i = 1; i < argc; i++) {
		char buf[512];
		int ret, r = open(argv[i], O_RDONLY);
		if (r == -1) {
			perror(argv[i]);
			return -1;
		}
		while ((ret = read(r, buf, sizeof(buf)))) {
			if (ret == -1) {
				perror(argv[i]);
				return -1;
			}
			fwrite(buf, 1, sizeof(buf), w);
		}
		close(r);
	}
	fclose(w);
	return 0;
}
