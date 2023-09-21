#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "type.h"
#include "dbtype.h"

#define MAX_FD_NUM 64

typedef struct Fd {
    int fd;
    struct fd *freenext;
    struct fd *usenext;
} Fd;

typedef struct {
    Fd fdarr[MAX_FD_NUM];
    Fd *usehead;
    Fd *usetail;
    Fd *freehead;
} FdMgr;

typedef struct {
    FdMgr *fdmgr;
} DiskMgr;

typedef struct {
    Id tblspcid;
    Id dbid;
    Id relid;
} FilePath;

char *smgr_read(FilePath path, unsigned int pageno);
void smgr_write(FilePath path, unsigned int pageno, uchar *page);