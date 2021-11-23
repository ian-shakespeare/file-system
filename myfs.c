#include <inttypes.h>
#include <fcntl.h>

struct SuperBlock {
    uint64_t magic;
    uint64_t disk_size;
    uint64_t inodes;
};

int opendisk(char *filename, uint64_t size) {
    int fd = open("/mnt/e/CS/3400/fs/v.disk", O_RDWR);
    if (fd < 0) {
        fd = open("/mnt/e/CS/3400/fs/v.disk", O_RDWR | O_CREAT);
    }


    bad:
        return -1;
}

int readblock(int handle, uint64_t blocknum, void *buffer) {
    
}
int writeblock(int handle, uint64_t blocknum, void *buffer) {

}

int syncdisk(int handle) {

}

int closedisk(int handle) {

}