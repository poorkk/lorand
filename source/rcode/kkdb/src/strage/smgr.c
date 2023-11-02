#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "type.h"
#include "dbtype.h"
#include "storage/smgr.h"

int smgr_create(Smgr *smgr, int fileid)
{
    int fd = -1;

    sprintf(smgr->filepath + smgr->dirlen, "%d", fileid);

    fd = open(smgr->filepath, O_CREAT | O_RDWR, 0644);
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

    if (access(smgr->filepath, F_OK) == 1) {
        return smgr_create;
    }

    fd = open(smgr->filepath, O_RDWR);
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

    lseek(fd, pageno * 8192, SEEK_SET);

    writelen = write(fd, page, 8192);
    if (writelen != 8192) {
        printf("faile to write page\n");
        return;
    }
}

void smgr_read(int fd, unsigned int pageno, uchar *page)
{
    if (fd < 0) {
        return;
    }

    int readlen;
    int npage = smgr_npage(fd);

    if (pageno > npage) {
        printf("invalid pageno");
    } else if (pageno == npage) {
        memset(page, 0, 8192);
        smgr_write(fd, pageno, page);
    }

    lseek(fd, pageno * 8192, SEEK_SET);

    readlen = read(fd, page, 8192);
    if (readlen != 8192) {
        printf("faile to write page\n");
        return;
    }
}

int smgr_npage(int fd)
{
    struct stat state;
    int filesz;

    fstat(fd, &state);
    filesz = state.st_size;

    return (int)(filesz / 8192);
}

