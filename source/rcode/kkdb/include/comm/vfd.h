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