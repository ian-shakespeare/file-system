#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#define BLOCK_SIZE 4096

void handleerr(bool b, int handle);

int opendisk(char *filename, uint64_t size);
int readblock(int handle, uint64_t blocknum, void *buffer);
int writeblock(int handle, uint64_t blocknum, void *buffer);
int syncdisk(int handle);
int closedisk(int handle);
int formatdisk(int handle);
void dumpdisk(int handle);