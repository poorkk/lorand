#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page.h"

void page_init(Page *page)
{
    PageHeader *ph;

    memset(page, 0, PAGE_SIZE);

    ph = PAGE_GET_HDR(page);
    ph->headpos = sizeof(PageHeader);
    ph->tailpos = PAGE_SIZE;
    ph->datapos = PAGE_SIZE;
}

size_t page_get_free_size(Page *page)
{
    PageHeader *ph;
    size_t freesz;

    ph = PAGE_GET_HDR(page);
    freesz = ph->datapos - ph->headpos;

    return freesz > sizeof(ItemDesc) ? freesz - sizeof(ItemDesc) : 0;
}

void page_add_item(Page *page, const void *item, size_t itemlen)
{
    PageHeader *ph;
    size_t freesz;
    ItemDesc *itemdesc;

    freesz = page_get_free_size(page);
    if (freesz < itemlen) {
        return;
    }

    itemdesc = PAGE_GET_NEW_ITEM_DESC(page);
    itemdesc->len = itemlen;

    ph = PAGE_GET_HDR(page);
    ph->datapos -= itemlen;
    ph->headpos += sizeof(ItemDesc);

    memcpy(page + ph->datapos, item, itemlen);

    itemdesc->pos = ph->datapos;
}

void page_test()
{
    uchar *page = (uchar *)malloc(PAGE_SIZE);
    int itemnum;
    int i;

    const char *data[] = {
        "aaaaa1aaaaaaaaaa",
        "bbbbbbbbbbbbbbb2bbbbbbbbbbbbbbbbbbb",
        "ccccccc3cccccccccccccccccccccccccccccccc",
        "dddddddddddddd4ddddddddddddddddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee5eeeeeeeeeeeeeeeee"
    };

    page_init(page);

    for (i = 0; i < ARR_LEN(data); i++) {
        page_add_item(page, data[i], strlen(data[i]));
    }

    itemnum = PAGE_GET_ITEM_NUM(page);
    printf("item num: %d\n", itemnum);

    PageScan scan;
    page_scan_init(&scan, page);
    for (;;) {
        page_scan(&scan);
        if (scan.itemdata == NULL) {
            break;
        }
        printf("scan: %s\n", (char *)scan.itemdata);
    }
}