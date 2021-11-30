#include "myfs.h"

void handleerr(bool b, int handle) {
    if (!b) {
        printf("ERROR! closing disk\n");
        closedisk(handle);
        exit(-1);
    }
}

/* BLOCK INTERACTION */
int opendisk(char *filename, uint64_t size) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        fd = open(filename, O_RDWR | O_CREAT);
        ftruncate(fd, size);
    }
    assert(fd >= 0);
    if (fd >= 0)
        return fd;
    printf("open failed: %d\nerrno: %d\n", fd, errno);
    return -1;
}

int readblock(int handle, uint64_t blocknum, void *buffer) {
    if (lseek(handle, blocknum * BLOCK_SIZE, SEEK_SET) < 0) {
        perror("lseek error\n");
        return -1;
    }

    int r = read(handle, buffer, BLOCK_SIZE);
    if (r == BLOCK_SIZE)
        return 0;
    printf("read failed: %d\n", r);
    printf("errno: %d\n", errno);
    handleerr(false, handle);
    return -1;
}

int writeblock(int handle, uint64_t blocknum, void *buffer) {
    if (lseek(handle, blocknum * BLOCK_SIZE, SEEK_SET) < 0) {
        perror("lseek error\n");
        return -1;
    }

    int w = write(handle, buffer, BLOCK_SIZE);
    if (w >= 0)
        return 0;
    printf("write failed: %d\n", w);
    printf("errno: %d\n", errno);
    handleerr(false, handle);
    return -1;
}

int syncdisk(int handle) {
    int f = fsync(handle);
    handleerr(f >= 0, handle);
    return f;
}

int closedisk(int handle) {
    int c = close(handle);
    assert(c >= 0);
    return c;
}

bool testbit(int n) {
    uint64_t index = n / (8 * sizeof(uint64_t));
    uint64_t offset = n % (8 * sizeof(uint64_t));
    uint64_t mask = 1 << offset;

    /* test if the bit is set */
    if (freeblocks[index] & mask)
        return true;
    return false;
}

void setbit(int n) {
    uint64_t index = n / (8 * sizeof(uint64_t));
    uint64_t offset = n % (8 * sizeof(uint64_t));
    uint64_t mask = 1 << offset;

    /* set the bit */
    freeblocks[index] |= mask;
}

void clearbit(int n) {
    uint64_t index = n / (8 * sizeof(uint64_t));
    uint64_t offset = n % (8 * sizeof(uint64_t));
    uint64_t mask = 1 << offset;

    /* clear the bit */
    freeblocks[index] &= ~mask;
}

int formatdisk(int handle) {
    char* super[BLOCK_SIZE];
    super[0] = (char*)0xDEADBEEF;
    super[1] = (char*)(BLOCK_SIZE * 64);
    super[2] = (char*)128;
    writeblock(handle, 0, &super);

    /* CLEAR AND WRITE THE BITMAP */
    for (int i = 0; i < 512; i++) {
        freeblocks[i] = 0;
    }
    setbit(0);
    setbit(1);

    writeblock(handle, 1, &freeblocks);
    syncdisk(handle);
    return 0;
}

void dumpdisk(int handle) {
    char* super[BLOCK_SIZE];
    readblock(handle, 0, &super);

    int total = 0;
    printf("########### DISK DUMP ###########\n");
    for (int i = 2; i < (int64_t) super[2]; i++) {
	if (testbit(i)) {
		printf("%d\n", i);
		total++;
	}
    }
    printf("# magic: %lx\n# disk size: %ldkb\n# num of inodes: %ld\n# active inodes: %d\n", (uint64_t) super[0], ((int64_t) super[1]) / 1024, (int64_t) super[2], total);
    printf("#################################\n");
}

int createfile(int handle, uint64_t sz, uint64_t t) {
	char* buf[BLOCK_SIZE];
	readblock(handle, 0, &buf);

	uint64_t nodes[509];
	int used = 0;
	for (uint64_t i = 2; i < (uint64_t) buf[2] && used <= (sz / BLOCK_SIZE); i++) {
		if (!testbit(i)) {
			nodes[used] = i;
			setbit(i);
			used++;
		}
	}
	printf("fsize: %ld, blocks used: %d\n", sz, used);
	handleerr((nodes[0] >= 2), handle);
	struct inode n;
	n.size = sz;
	n.mtime = time(NULL);
	n.type = t;
	for (int i = 1; i <= (sz / BLOCK_SIZE); i++) {
		n.blocks[i-1] = nodes[i];
	}

	writeblock(handle, nodes[0], &n);
	writeblock(handle, 1, &freeblocks);
	syncdisk(handle);

	return nodes[0];
}

void deletefile(int handle, uint64_t blocknum) {
	struct inode n;
	readblock(handle, blocknum, &n);
	clearbit(blocknum);

    for (int i = 0; i < (n.size / BLOCK_SIZE); i++) {
	clearbit(n.blocks[i]);
    }

    writeblock(handle, 1, &freeblocks);
    syncdisk(handle);
}
