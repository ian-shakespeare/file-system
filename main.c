#include "myfs.h"

int main() {
    printf("=================================\n");
    /* OPEN VIRTUAL DISK */
    printf("Opening Virtual Disk...");
    char* path = "./v.disk";
    int handle = opendisk(path, BLOCK_SIZE * 16);
    printf("Successful\n");

    char* super[BLOCK_SIZE];
    readblock(handle, 0, &super);
    if ((uint64_t) super[0] != 0xDEADBEEF) {
        printf("Formatting Disk...");
        formatdisk(handle);
        printf("Successful\n");
    }
    printf("=================================\n");

    dumpdisk(handle);
    printf("=================================\n");
    printf("         BEGINNING TEST 1\n");
    /* TEST SINGLE BLOCK WRITE AND READ */
    // create and write buf
    char* buf[BLOCK_SIZE];
    buf[0] = "hello";
    printf("%s\n", buf[0]);
    int r = writeblock(handle, 2, &buf);
    handleerr(r >= 0, handle);
    // clear buf
    buf[0] = "";
    printf("%s\n", buf[0]);
    // read block
    r = readblock(handle, 2, &buf);
    handleerr(r >= 0, handle);
    printf("%s\n", buf[0]);


    /* CLOSE VIRTUAL DISK */
    syncdisk(handle);

    printf("Closing Virtual Disk...");
    closedisk(handle);
    printf("Successful\n");
    exit(0);
}