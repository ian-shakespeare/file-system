#include "myfs.h"

int main() {
    printf("=================================\n");
    /* OPEN VIRTUAL DISK */
    printf("Opening Virtual Disk...");
    char* path = "./v.disk";
    int handle = opendisk(path, BLOCK_SIZE * 64);
    printf("Successful\n");

    uint64_t super[BLOCK_SIZE];
    readblock(handle, 0, super);
    if (super[0] != 0xDEADBEEF) {
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
    /*char* buf[BLOCK_SIZE];
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
    printf("%s\n", buf[0]);*/

    printf("creating file...\n");
    int f1 = createfile(handle, 27000, 0);
    dumpdisk(handle);
    dumpfile(handle, f1);
    printf("enlarging file...\n");
    enlargefile(handle, f1, 4000);
    dumpdisk(handle);
    printf("enlarging more...\n");
    enlargefile(handle, f1, 12000);
    dumpdisk(handle);
    printf("shrinking file...\n");
    shrinkfile(handle, f1, 12000);
    dumpdisk(handle);
    printf("deleting...\n");
    deletefile(handle, f1);
    dumpdisk(handle);

    printf("=================================\n");
    printf("         BEGINNING TEST 2\n");

    printf("creating file...\n");
    int f2 = createfile(handle, 200, 0);
    char* word = "Hello!\n";
    printf("writing file...\n");
    writefile(handle, f2, word, 8);
    char newword[8];
    printf("reading file...\n");
    readfile(handle, f2, newword, 8);
    printf("%s\n", newword);
    deletefile(handle, f2);

    /* CLOSE VIRTUAL DISK */
    syncdisk(handle);
    printf("=================================\nClosing Virtual Disk...");
    closedisk(handle);
    printf("Successful\n");
    printf("=================================\n");
    exit(0);
}
