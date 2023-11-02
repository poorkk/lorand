#ifndef SMGR_H_
#define SMGR_H_

typedef struct {
    const char *datadir;
    int dirlen;
    char filepath[2048];
} Smgr;

int smgr_create(Smgr *smgr, int fileid);
int smgr_open(Smgr *smgr, int fileid);
void smgr_write(int fd, unsigned int pageno, uchar *page);
void smgr_read(int fd, unsigned int pageno, uchar *page);
int smgr_npage(int fd);

#endif
