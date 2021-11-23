#include <inttypes.h>
#include <fcntl.h>

struct SuperBlock {
    uint64_t magic;
    uint64_t disk_size;
    uint64_t inodes;
};


/* BLOCK INTERACTION */
int opendisk(char *filename, uint64_t size) {
    int fd = open("./v.disk", O_RDWR | O_CREAT);
    ftruncate(fd, size);
    return fd;
}

int readblock(int handle, uint64_t blocknum, void *buffer) {
    lseek(handle, blocknum * 4096, SEEK_SET);
    int r = read(handle, buffer, 4096);
    lseek(handle, 0, SEEK_SET);
    return r;
}

int writeblock(int handle, uint64_t blocknum, void *buffer) {
    lseek(handle, blocknum * 4096, SEEK_SET);
    int w = write(handle, buffer, 4096);
    lseek(handle, 0, SEEK_SET);
    return w;
}

int syncdisk(int handle) {
    int f = fsync(handle);
    return f;
}

int closedisk(int handle) {
    int c = close();
    return c;
}

int formatdisk(int handle) {
    struct SuperBlock sb = { 0xDEADBEEF, 512, 0 };
}
