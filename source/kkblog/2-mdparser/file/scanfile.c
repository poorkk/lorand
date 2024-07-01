#define bool char
#define true 1
#define false 0

#define PAGE_SIZE 8192
typedef struct {
    const char *filepath;
    int fd;

    int curpage;
    int maxpage;
    char curpage[PAGE_SIZE];

    char curline[PAGE_SIZE];
} MdFileScan;

MdFileScan *md_scan_init(const char *file)
{
    MdFileScan *scan = (MdFileScan *)malloc(sizeof(MdFileScan));
    if (scan == NULL) {
        ;
    }
    scan->filepath = file;
    ;
}