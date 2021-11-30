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
    super[1] = (char*)(BLOCK_SIZE * 16);
    super[2] = (char*)0;
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
    printf("########### DISK DUMP ###########\n");
    printf("# magic: %lx\n# disk size: %ldkb\n# num of inodes: %ld\n", (uint64_t) super[0], ((int64_t) super[1]) / 1024, (int64_t) super[2]);
    printf("#################################\n");
}

int createfile(int handle, uint64_t sz, uint64_t t) {
    if (sz < BLOCK_SIZE) {
        struct inode node;
        node.size = sz;
        node.mtime = time(NULL);
        node.type = t;
        int i;
        for (i = 2; i < 512; i++) {
            if (!testbit(i))
                break;
        }
        setbit(i);

        char* buf[BLOCK_SIZE];
        readblock(handle, 0, &buf);
        buf[2]++;

        writeblock(handle, 0, &buf);
        writeblock(handle, 1, &freeblocks);
        writeblock(handle, i, &node);
        syncdisk(handle);
        return i;
    }
    else
        assert(false);
        return -1;
}

void deletefile(int handle, uint64_t blocknum) {
    clearbit(blocknum);

    char* buf[BLOCK_SIZE];
    readblock(handle, 0, &buf);
    buf[2]--;

    writeblock(handle, 0, &buf);
    writeblock(handle, 1, &freeblocks);
    syncdisk(handle);
}