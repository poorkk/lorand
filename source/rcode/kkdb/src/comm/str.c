#include "comm/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * buf: xxxeaaa
 * end: e, skip: a
 * return true
 */
bool str_end_with(const char *buf, int buflen, char end, char skip)
{
    if (buflen == 0) {
        return false;
    }

    for (int i = buflen - 1; i >= 0; i--) {
        if (buf[i] == skip) {
            continue;
        }
        if (buf[i] == end) {
            return true;
        }
        return false;
    }
    return false;
}

const char *str_spilt(const char *data, char split, char *buf, size_t bufsz)
{
    const char *end;
    size_t bufuse = 0;

    if (data == NULL) {
        return NULL;
    }

    for (end = data; end[0] != '\0' && end[0] == ' '; end++) {}; /* skip whitespace at the head */
    for (; end[0] != '\0'; end++) {
        if (end[0] == split || (bufuse == bufsz - 1)) {
            break;
        }
        buf[bufuse++] = end[0];
    }
    for (; bufuse > 0 && buf[bufuse - 1] == ' '; bufuse--) {}; /* skip whitespace at the tail  */
    buf[bufuse] = '\0';

    if (end[0] != '\0') {
        return end + 1;
    }
    return bufuse > 0 ?  end : NULL;
}

void str_test()
{
    const char *a = "abc, 123, 456   , 8910, ";
    const char *cur = a;
    for (;;) {
        char buf[20];
        cur = str_spilt(cur, ' ', buf, 20);
        if (cur != NULL) {
            printf("%s\n", buf);
        } else {
            break;
        }
    }
}

char *kd_strstr(char *buf, int buflen, const char *find)
{
    int findlen = strlen(find);
    int start;
    int mlen; /* match len */

    for (start = 0; start < buflen; start++) {
        if (buf[start] == find[0]) {
            for (mlen = 1; mlen < buflen; mlen++) {
                if  (mlen >= findlen) {
                    return buf + start;
                }

                if (buf[start + mlen] != find[mlen]) {
                    break;
                }
            }
        }
    }

    return NULL;
}

KdBuf *buf_malloc(int bufsz)
{
    KdBuf *buf = (KdBuf *)malloc(sizeof(KdBuf));
    if (buf == NULL) {
        return NULL;
    }

    buf->buf = (char *)malloc(bufsz);
    if (buf->buf == NULL) {
        free(buf);
        return NULL;
    }
    memset(buf->buf, 0, bufsz);

    buf->size = bufsz;
    buf->used = 0;
    return buf;
}

void buf_write(KdBuf *buf, char *data, int datalen)
{
    if (buf->used + datalen > buf->size) {
        return;
    }

    memcpy(buf->buf + buf->used, data, datalen);
    buf->used += datalen;
}

void buf_write_str(KdBuf *buf, const char *str)
{
    size_t slen = strlen(str);

    if (buf->used + slen > buf->size) {
        return;
    }
    strcat(buf->buf, str);
    buf->used += slen;
}

void buf_reset(KdBuf *buf)
{
    memset(buf->buf, 0, buf->used);
    buf->used = 0;
}

