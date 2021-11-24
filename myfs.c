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
    if (fd < 0)
        fd = open(filename, O_RDWR | O_CREAT);
    assert(fd >= 0);
    ftruncate(fd, size);
    if (fd >= 0)
        return fd;
    printf("open failed: %d\nerrno: %d\n", fd, errno);
    return -1;
}

int readblock(int handle, uint64_t blocknum, void *buffer) {
    lseek(handle, blocknum * 4096, SEEK_SET);
    int r = read(handle, buffer, 4096);
    lseek(handle, 0, SEEK_SET);

    if (r >= 0)
        return 0;
    printf("read failed: %d\n", r);
    printf("errno: %d\n", errno);
    return -1;
}

int writeblock(int handle, uint64_t blocknum, void *buffer) {
    lseek(handle, blocknum * 4096, SEEK_SET);
    int w = write(handle, buffer, 4096);
    lseek(handle, 0, SEEK_SET);
    
    if (w >= 0)
        return 0;
    printf("write failed: %d\n", w);
    printf("errno: %d\n", errno);
    return -1;
}

int syncdisk(int handle) {
    int f = fsync(handle);
    return f;
}

int closedisk(int handle) {
    int c = close(handle);
    assert(c >= 0);
    return c;
}

int formatdisk(int handle) {
    struct SuperBlock sb = { 0xDEADBEEF, 4096 * 16, 0 };
    writeblock(handle, 0, &sb);

    /* CLEAR AND WRITE THE BITMAP */
    /*for (int i = 0; i < 512; i++) {
        freeblocks[i] = 0;
    }*/
    writeblock(handle, 1, &freeblocks);
    return 0;
}

void dumpdisk(int handle) {
    struct SuperBlock sb;
    readblock(handle, 0, &sb);
    printf("disk_size: %d\n", (int) sb.disk_size);
}