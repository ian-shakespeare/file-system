#include "myfs.h"

uint64_t freeblocks[512];

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

/* BIT MANIPULATION */
bool testbit(uint64_t n) {
    uint64_t index = n / 64;//(8 * sizeof(uint64_t));
    uint64_t offset = n % 64;//(8 * sizeof(uint64_t));
    uint64_t mask;
    if (offset < 32)
        mask = 1 << offset;
    else {
        mask = 1 << 31;
        mask = mask << 1;
        mask = mask << (offset - 32);
    }

    /* test if the bit is set */
    if (freeblocks[index] & mask)
        return true;
    return false;
}

void setbit(uint64_t n) {
    uint64_t index = n / 64;//(8 * sizeof(uint64_t));
    uint64_t offset = n % 64;//(8 * sizeof(uint64_t));
    uint64_t mask;
    if (offset < 32)
        mask = 1 << offset;
    else {
        mask = 1 << 31;
        mask = mask << 1;
        mask = mask << (offset - 32);
    }

    /* set the bit */
    freeblocks[index] |= mask;
} 

void clearbit(uint64_t n) {
    uint64_t index = n / 64;//(8 * sizeof(uint64_t));
    uint64_t offset = n % 64;//(8 * sizeof(uint64_t));
    uint64_t mask;
    if (offset < 32)
        mask = 1 << offset;
    else {
        mask = 1 << 31;
        mask = mask << 1;
        mask = mask << (offset - 32);
    }

    /* clear the bit */
    freeblocks[index] &= ~mask;
}

/* DISK FORMATTING AND DUMPING */
int formatdisk(int handle) {
    // CREATE AND WRITE SUPER BLOCK
    uint64_t super[BLOCK_SIZE];
    super[0] = 0xDEADBEEF;
    super[1] = BLOCK_SIZE * 64;
    super[2] = INODES;
    writeblock(handle, 0, super);

    // CLEAR AND WRITE THE BITMAP
    handleerr(!testbit(32), handle);
    setbit(0);
    setbit(1);
    for (int i = 2; i < INODES; i++) {
        clearbit(i);
    }
    assert(testbit(0));
    writeblock(handle, 1, freeblocks);

    syncdisk(handle);
    return 0;
}

void dumpdisk(int handle) {
    uint64_t super[BLOCK_SIZE];
    readblock(handle, 0, super);

    int total = 0;
    printf("########### DISK DUMP ###########\n");
    char inodechar[INODES];
    for (uint64_t i = 0; i < INODES; i++) {
        if (testbit(i)) {
            total++;
            inodechar[i] = '@';
        }
        else
            inodechar[i] = '-';
    }
    printf("# magic: %lx\n# disk size: %ldkb\n# num of inodes: %ld\n# active inodes: %d\n", (uint64_t) super[0], ((int64_t) super[1]) / 1024, (int64_t) super[2], total);
    printf("inodes = [ %s ]\n", inodechar);
    printf("#################################\n");
}

/* FILE INTERACTION */
int createfile(int handle, uint64_t sz, uint64_t t) {
	struct inode n;
	n.size = sz;
	n.mtime = time(NULL);
	n.type = t;
	uint64_t used = 0;
	for (uint64_t i = 2; i < INODES && used <= (sz / BLOCK_SIZE); i++) {
		if (!testbit(i)) {
			n.blocks[used] = i;
			setbit(i);
            setbit(i + INODES);
			used++;
		}
	}

	writeblock(handle, n.blocks[0], &n);
	writeblock(handle, 1, freeblocks);
	syncdisk(handle);

	return n.blocks[0];
}

void deletefile(int handle, uint64_t blocknum) {
	struct inode n;
	readblock(handle, blocknum, &n);
	clearbit(blocknum);
    clearbit(blocknum + INODES);

    for (uint64_t i = 0; i < (n.size / BLOCK_SIZE); i++) {
	    clearbit(n.blocks[i + 1]);
        clearbit(n.blocks[i + 1] + INODES);
    }

    writeblock(handle, 1, freeblocks);
    syncdisk(handle);
}

void dumpfile(int handle, uint64_t blocknum) {
    struct inode node;
    readblock(handle, blocknum, &node);
    printf("/////////// FILE DUMP ///////////\n");
    printf("// size: %ld\n", node.size);
    printf("// time: %ld\n", node.mtime);
    if (!node.type)
        printf("// type: regular\n");
    else
        printf("// type: directory\n");
    printf("// inode location: %ld\n", node.blocks[0]);
    printf("/////////////////////////////////\n");
}

int enlargefile(int handle, uint64_t blocknum, uint64_t sz) // sz is the amount you would like to INCREASE by, i.e. node.size += sz
{
    // Calculate the blocks currently being used by the node, and the blocks needed for enlarging
    struct inode node;
    readblock(handle, blocknum, &node);
    uint64_t blocks_used = node.size / BLOCK_SIZE;
    uint64_t blocks_needed = (node.size + sz)/ BLOCK_SIZE;

    // If no more blocks are needed, update node size and save
    if (blocks_used == blocks_needed) {
	    node.size += sz;
	    writeblock(handle, blocknum, &node);
        return node.size;
    }

    // If blocks are needed, begin adding new blocks to node starting where the used blocks end
    uint64_t assigning = blocks_used + 1;
    for (uint64_t i = 2; i < INODES && assigning <= (blocks_needed); i++) {
		if (!testbit(i)) {
            // assign block, set bits and continue
			node.blocks[assigning] = i;
			setbit(i);
			setbit(i + INODES);
			assigning++;
		}
	}
    // update node and save
    node.size += sz;
    writeblock(handle, blocknum, &node);
    syncdisk(handle);
    return node.size;
}

int shrinkfile(int handle, uint64_t blocknum, uint64_t sz) // sz is the amount you would like to DECREASE by, i.e. node.size -= sz
{
    // Works same as enlarge, but in reverse
	struct inode node;
	readblock(handle, blocknum, &node);
	uint64_t blocks_used = node.size / BLOCK_SIZE;
	uint64_t blocks_needed = (node.size - sz)/ BLOCK_SIZE;

	handleerr(sz < node.size, handle);

	if (blocks_used == blocks_needed) {
		node.size -= sz;
		writeblock(handle, blocknum, &node);
		return node.size;
	}

	for (uint64_t i = blocks_used; i > blocks_needed; i--) {
		clearbit(node.blocks[i]);
		clearbit(node.blocks[i] + INODES);
	}
	node.size -= sz;
	writeblock(handle, blocknum, &node);
    syncdisk(handle);
	return node.size;
}

/*int writefile(int handle, uint64_t blocknum, void *buffer) {
	handleerr(testbit(blocknum), handle);

	struct inode node;
	readblock(handle, blocknum, &node);

    uint64_t bl[BLOCK_SIZE];

	if ((node.size / BLOCK_SIZE) >= sizeof(buffer)) {
		writeblock(handle, blocknum + INODES, buffer);
		syncdisk(handle);
		return sizeof(buffer);
	}
	else if (sizeof(buffer) < node.size) {
	}
    else {
    }

    return 0;
}

int readfile(int handle, uint64_t blocknum, void *buffer) {
    handleerr(testbit(blocknum), handle);

    struct inode node;
    readblock(handle, blocknum, &node);

    for (uint64_t i = 0; i < node.size / BLOCK_SIZE; i++) {
        readblock(handle, node.blocks[0] + INODES, buffer + (i * BLOCK_SIZE));
    }

    return 0;
}
*/