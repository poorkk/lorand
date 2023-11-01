#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "type.h"
#include "dbtype.h"

typedef struct {
    const char *datadir;
    int dirlen;
    char filepath[2048];
} Smgr;

int smgr_create(Smgr *smgr, int fileid)
{
    int fd = -1;

    sprintf(smgr->filepath + smgr->dirlen, "%d", fileid);

    fd = open(smgr->filepath, O_CREAT);
    if (fd < 0) {
        printf("failed to create file '%s'\n", smgr->filepath);
        return fd;
    }

    return fd;
}

int smgr_open(Smgr *smgr, int fileid)
{
    int fd = -1;

    sprintf(smgr->filepath + smgr->dirlen, "%d", fileid);

    fd = open(smgr->filepath, O_CREAT);
    if (fd < 0) {
        printf("failed to create file '%s'\n", smgr->filepath);
        return fd;
    }

    return fd;
}

void smgr_write(int fd, unsigned int pageno, uchar *page)
{
    if (fd < 0) {
        return;
    }

    int writelen;

    // fseek

    writelen = write(fd, page, 8192);
    if (writelen != 8192) {
        printf("faile to write page\n");
        return;
    }
}



