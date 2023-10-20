#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

int kd_file_read_all(const char *file, char *buf, int bufsz)
{
    int readlen = 0;
    int fd;
    struct stat state;
    int filesz;

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        printf("failed to read file '%s'\n", file);
        return 0;
    }

    fstat(fd, &state);
    filesz = state.st_size;

    if (bufsz < filesz) {
        printf("failed to read file, the dest buffer is too small.\n");
        return 0;
    }

    readlen = read(fd, buf, filesz);

    return readlen;
}

void file_scan(const char *file)
{
#define BUF_SZ 4096

    char buf[BUF_SZ];

    int fd = open(file, O_RDONLY);
    int cnt;
    
    for (;;) {
        int readlen = read(fd, buf, BUF_SZ);
        printf("%d\n", readlen);
        if (readlen <= 0) {
            break;
        }
    }
    close(fd);
}