#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "file.h"
#include "common.h"

int create_db_file(const char* const filename)
{
    int fd = open(filename, O_RDONLY); // if the file can be opened it means the file already exists
    if (fd != -1) { fprintf(stderr, "Database file %s already exists\n", filename); close(fd); return STATUS_ERROR; }
    
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) { fprintf(stderr, "Unable to open file %s: %s\n", filename, strerror(errno)); return STATUS_ERROR; }

    return fd;
}

int open_db_file(const char* const filename)
{
    int fd = open(filename, O_RDWR, 0644);
    if (fd == -1) { fprintf(stderr, "Unable to open file %s: %s\n", filename, strerror(errno)); return STATUS_ERROR; }

    return fd;
}