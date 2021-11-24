#include "myfs.h"

int main() {
    /* OPEN VIRTUAL DISK */
    printf("Opening Virtual Disk...");
    char* path = "./v.disk";
    int handle = opendisk(path, 4096 * 16);
    printf("Successful\n");

    struct SuperBlock sb;
    readblock(handle, 0, &sb);
    if (sb.magic != 0xDEADBEEF)
        formatdisk(handle);

    dumpdisk(handle);

    /* TEST SINGLE BLOCK WRITE AND READ */
    char* buf = "hello";
    printf("%s\n", buf);
    int r = writeblock(handle, 2, &buf);
    handleerr(r >= 0, handle);

    buf = "";
    printf("%s\n", buf);

    r = readblock(handle, 2, &buf);
    handleerr(r >= 0, handle);
    printf("%s\n", buf);

    /* CLOSE VIRTUAL DISK */
    printf("Closing Virtual Disk...");
    closedisk(handle);
    printf("Successful\n");
    exit(0);
}