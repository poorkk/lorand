#ifndef PAGE_H_
#define PAGE_H_

#include <stdlib.h>
#include "type.h"

typedef struct {
    uint16 pos;
    uint16 len;
} ItemDesc;

/*
 * +--------+
 * | header |
 * +--------+ <- headpos
 * | free   |
 * +--------+ <- datapos
 * | data   |
 * +--------+ <- tailpos
 * | tail   |
 * +--------+
 */

typedef struct {
    uint16 headpos;
    uint16 datapos;
    uint16 tailpos;

    ItemDesc descarr[VAR_ARR_SIZE];
} PageHeader;

typedef uchar Page;
#define PAGE_SIZE 8192
#define PAGE_GET_HDR(page) ((PageHeader *)(page))
#define PAGE_GET_NEW_ITEM_DESC(page) (ItemDesc *)((uchar *)(page) + PAGE_GET_HDR((page))->headpos)
#define PAGE_GET_ITEM_NUM(page) ((PAGE_GET_HDR((page))->headpos - sizeof(PageHeader)) / sizeof(ItemDesc))

void page_init(Page *page);
size_t page_get_free_size(Page *page);
void page_add_item(Page *page, const void *item, size_t itemlen);

typedef struct {
    Page *page;
    ItemDesc *itemdesc;
    uchar *itemdata;
    uint16 curpos;
} PageScan;

#define page_scan_init(scan, page)       \
do {                                     \
    (scan)->page = (page);               \
    (scan)->itemdesc = NULL;             \
    (scan)->itemdata = NULL;             \
    (scan)->curpos = sizeof(PageHeader); \
} while (0)

#define page_scan(scan)                                                 \
do {                                                                    \
    if ((scan)->curpos >= PAGE_GET_HDR((scan)->page)->headpos) {        \
        (scan)->itemdata = NULL;                                        \
    } else {                                                            \
        (scan)->itemdesc = (ItemDesc *)((scan)->page + (scan)->curpos); \
        (scan)->itemdata = (scan)->page + (scan)->itemdesc->pos;        \
        (scan)->curpos += sizeof(ItemDesc);                             \
    }                                                                   \
} while (0)


void page_test();

#endif