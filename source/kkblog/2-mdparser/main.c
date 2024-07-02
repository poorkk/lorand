#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Lnode {
    char *eng_name;
    char *chn_name;

    int fd;
    size_t file_sz;
} MdFile;

typedef struct {
    char *eng_name;
    char *chn_name;

    int file_num;
    MdFile *files;
} MdDir;

MdDir *md_dir_find_files(const char *md_dir);

typedef struct {
    char *root_dir;

    int dir_num;
    MdDir *dirs;
} MdRootDir;

MdRootDir *md_root_dir_find_files(const char *md_root);

#define PAGE_SIZE 8192
#define bool char
#define true 1
#define false 0
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    MdFile md_file;

    int cur_page;
    int max_page;
    char cur_page[PAGE_SIZE];

    /* if the length of the markdown object is more than 'PAGE_SIZE', we need realloc a larger buffer to store it */
    size_t obj_len;
    char cur_obj[PAGE_SIZE];
    char *large_obj;
} MdGroupScan;

typedef enum {
    MD_NONE = 0,
    MD_END,     /* \n */

    MD_H1,      /* #_\n */
    MD_H2,      /* ##_\n */
    MD_H3,      /* ###_\n */
    MD_H4,      /* ####_\n */
    MD_H5,      /* #####_\n */
    MD_H6,      /* ######_\n */

    MD_NLIST,   /* 1. _\n\n */
    MD_PLIST,   /* - _\n\n */

    MD_PICTURE,     /* []\n */
    MD_CODE,        /* ```_``` */
    MD_TABLE,       /* |_|\n|-|\n_\n\n */

    MD_TEXT,       
} MdObjType;

MdObjType md_line_get_type(char *line)
{
    if (line == NULL || strlen(line) == 0) {
        return MD_NONE;
    }

    int i;
    if (line[0] == '#') {
        ;
    }
}

MdGroupScan *md_scan_init(const char *md_file);
char *md_scan_group(MdGroupScan *scan);
{
    MdObjType cur;
    
    for (;;) {
        char *line;
        switch ()
    }
}
void md_scan_free(MdGroupScan **scan);

#define BUF_RESIZE 16384

typedef struct {
    char *buf;
    size_t size;
    size_t used;
} Buf;

Buf *buf_alloc();
void buf_free(Buf **buf);

void buf_cat(Buf *buf, char *data)
{
    size_t len = strlen(data) + 1;
    if (len > buf->size - buf->used) {
        buf->buf = (char *)realloc(buf->buf, buf->size + MAX(BUF_RESIZE, len));
    }
    buf->used += strcpy(buf->buf + buf->used, data);
}

void md_obj_to_html(char *md_obj, Buf *html_buf)
{

}

typedef struct {
    char *header;

    char *type_hdr;
    char *type_tail;

    char *list_hdr;
    char *list_tail;

    char *blog_hdr;
    char *blog_tail;

    char *tail;
} HtmlTemp;

typedef struct {
    HtmlTemp *temp;
} HtmlGenerater;

int main()
{
    char *md_root = NULL;
    char *html_root = NULL;
    int i;
    int j;

    /* find all markdown directories and files under the root path  */
    MdRootDir *md_dirs = md_root_dir_find_files(md_root);

    /* translate the markdown files into html files */
    for (i = 0; i < md_dirs->dir_num; i++) {
        MdDir *md_dir = &md_dirs->dirs[i];

        for (j = 0; j < md_dir->file_num; j++) {
            MdFile *md_file = &md_dir->files[j];
        }
    }
}