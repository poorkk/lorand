#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool char
#define true 1
#define false 0

/*
 * | H | Q 10           | T |
 */

typedef struct {
    char *buf;
    size_t used;
    size_t total;
} MsgBuf;

#define MBUF_GET_TAIL(mbuf) ((mbuf)->buf + (mbuf)->used)

typedef struct {
    char type;
} MsgHead;

typedef struct {
    MsgHead type;
    unsigned short len;
} DataHead;

#define MBUF_GET_TAIL_DATA(mbuf) (MBUF_GET_TAIL((mbuf)) + sizeof(DataHead))

/*
 * Msg Type:
 *      N   null
 *      H   head
 *      T   tail
 * 
 *      q   query
 *      r   result
 */

bool is_data_type(char type)
{
    char datatype[] = "qr";
    return (strchr(datatype, type) != NULL ? true : false);
}



void mbuf_put_msg(MsgBuf *buf, char type)
{
    MsgHead *tail = (MsgHead *)MBUF_GET_TAIL(buf);
    buf->used += sizeof(MsgHead);

    tail->type = type;
}

void mbuf_put_data(MsgBuf *buf, char type, const char *data, size_t datalen)
{
    DataHead *tail = (DataHead *)MBUF_GET_TAIL(buf);
    buf->used += sizeof(DataHead);

    tail->type.type = type;

    memcpy(MBUF_GET_TAIL(buf), data, datalen);
    buf->used += datalen;
}

void mbuf_init(MsgBuf *mbuf, char *buf, size_t bufsz)
{
    mbuf->buf = buf;
    mbuf->total = bufsz;
    mbuf->used = 0;

    mbuf_put_msg(mbuf, 'H');
}

void mbuf_end(MsgBuf *mbuf)
{
    mbuf_put_msg(mbuf, 'T');
}

typedef struct {
    MsgBuf *mbuf;
    size_t curlen;
    char *cur;

    char type;
    char *data;
    size_t datalen;
} MsgBufScan;

void mbuf_scan(MsgBufScan *scan)
{
    if (scan->curlen >= scan->mbuf->used) {
        scan->type = 'N';
        return;
    }

    MsgHead *tail = (MsgHead *)scan->cur;
    scan->type = tail->type;

    if (is_data_type(scan->type)) {
        scan->datalen = ((DataHead *)tail)->len;
        scan->data = MBUF_GET_TAIL_DATA(scan->mbuf);

        scan->curlen += sizeof(DataHead);
        scan->curlen += scan->datalen;
    } else {
        scan->curlen += sizeof(MsgHead);
    }

    scan->cur += scan->curlen;

    if (scan->type == 'H') {
        mbuf_scan(scan);
    }
}

void test()
{
    const char *arr[] = {
        "liyanxuan ",
        "shi ",
        "yi ",
        "ge",
        "xiao",
        "bao",
        "bei"
    };

    MsgBuf m = {0};
    m.buf = (char *)malloc(1000);

    mbuf_init(&m, m.buf, 1000);

    size_t i;
    for (i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
        mbuf_put_data(&m, 'D', arr[i], strlen(arr[i]) + 1);
    }

    mbuf_end(&m);

    MsgBufScan scan = {0};
    scan.mbuf = &m;

    for (;;) {
        mbuf_scan(&scan);

        if (scan.type == 'T' || scan.type == 'N') {
            break;
        }

        printf("[%lu] %s\n", scan.datalen, scan.data);
    }
}

int main()
{
    test();
}