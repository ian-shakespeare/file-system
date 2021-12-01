#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define INODES 128

struct inode {
    uint64_t size;
    uint64_t mtime;
    uint64_t type;
    uint64_t blocks[509];
};

void handleerr(bool b, int handle);

int opendisk(char *filename, uint64_t size);
int readblock(int handle, uint64_t blocknum, void *buffer);
int writeblock(int handle, uint64_t blocknum, void *buffer);
int syncdisk(int handle);
int closedisk(int handle);
bool testbit(int n);
void setbit(int n);
void clearbit(int n);
int formatdisk(int handle);
void dumpdisk(int handle);
int createfile(int handle, uint64_t sz, uint64_t t);
void deletefile(int handle, uint64_t blocknum);
