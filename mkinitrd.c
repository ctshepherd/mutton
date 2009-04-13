#include <stdint.h>
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
	unsigned offset, dir_num = 1;
	FILE *w = fopen("initrd.img", "w");
	if (!w) {
		perror("initrd.img");
		return -1;
	}

	uint32_t version = SUPPORTED_VERSION, chksum = SUPPORTED_VERSION^INITRD_MAGIC;
	fwrite(&version, sizeof(uint32_t), 1, w);
	fwrite(&chksum,  sizeof(uint32_t), 1, w);

	offset = argc*sizeof(struct initrd_inode);
	struct initrd_inode inode;
	inode.mask = S_IFDIR|S_IRUSR;
	inode.uid = inode.gid = 0;
	inode.ino = 0;
	inode.length = 0;
	inode.offset = 0;
	fwrite(&inode, sizeof(struct initrd_inode), 1, w);
	/* This still doesn't handle subdirectories, but can actually write
	 * empty directories to disk, as well as the root inode */
	for (i = 1; i < argc; i++) {
		struct stat buf;
		if (stat(argv[i], &buf) == -1) {
			perror(argv[i]);
			return -1;
		}
		inode.mask = buf.st_mode & ~(S_IWOTH|S_IWGRP|S_IWUSR); /* Unset writable permissions */
		if (buf.st_mode & S_IFDIR)
			dir_num++;
		inode.uid = buf.st_uid;
		inode.gid = buf.st_gid;
		inode.ino = i;
		inode.length = buf.st_size;
		inode.offset = offset;
		offset += buf.st_size;
		fwrite(&inode, sizeof(struct initrd_inode), 1, w);
	}
	memset(&inode, 0, sizeof(struct initrd_inode));
	fwrite(&inode, sizeof(struct initrd_inode), 1, w);

	fwrite(&dir_num, sizeof(uint32_t), 1, w);
	/* Write root inode dir */
	uint32_t root_ino = 0, num = argc-1;
	fwrite(&root_ino, sizeof(uint32_t), 1, w);
	fwrite(&num, sizeof(uint32_t), 1, w);
	for (i = 1; i < argc; i++)
		fwrite(&i, sizeof(uint32_t), 1, w);
	for (i = 1; i < argc; i++) {
		unsigned j, len;
		char n = 0;
		len = strlen(argv[i]);
		if (len > 127) {
			fprintf(stderr, "Filename '%s' too long\n", argv[i]);
			return -1;
		}
		fwrite(argv[i], 1, len, w);
		for (j = 0; j < 4-(len % 4); j++)
			fwrite(&n, 1, 1, w);
	}

	for (i = 1; i < argc; i++) {
		struct stat buf;
		uint32_t num;
		if (stat(argv[i], &buf) == -1) {
			perror(argv[i]);
			return -1;
		}
		if (!(buf.st_mode & S_IFDIR))
			continue;
		num = 0; /* Replace with num = buf.nlinks - 2 when we write subdirectories to disk */
		fwrite(&i, sizeof(uint32_t), 1, w);
		fwrite(&num, sizeof(uint32_t), 1, w);
	}

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
