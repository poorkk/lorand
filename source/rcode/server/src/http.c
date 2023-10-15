#include <stdio.h>
#include <string.h>
#include "str.h"
#include "http.h"

/*

http://192.168.3.23:7777/

GET / HTTP/1.1
Host: 192.168.3.23:7777
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*//*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Cookie: code-server-session=%24argon2id%24v%3D19%24m%3D65536%2Ct%3D3%2Cp%3D4%24Oh%2FJdUHvByQhC3dVW7cIDQ%24bzt67ui8zsN4xcNKHGYzCPIVoxXV2LPsejRx4kk63J8

 */

static KdHttp *http_new(KdBuf *buf)
{
#define HTTP_TMP_BUF_SZ 3072
    if (buf == NULL) {
        return NULL;
    }

    KdHttp *h = (KdHttp *)malloc(sizeof(KdHttp));
    if (h == NULL) {
        return NULL;
    }

    h->kbuf = buf;
    h->reqline = NULL;
    memset(h->methd, 0, sizeof(h->methd));
    h->reqhdr = NULL;
    h->reqbody = NULL;

    h->tmp = buf_malloc(HTTP_TMP_BUF_SZ);
    ASSERT(h->tmp != NULL);

    return h;
}

KdHttp *http_parse_reqmsg(KdBuf *buf)
{
#define MAX_HDR_KEY_LEN 128
    KdHttp *h = http_new(buf);
    if (h == NULL) {
        return NULL;
    }

    KdBuf *tmp = h->tmp;

    /* find reqline, reqhdr */
    h->reqline = h->kbuf->buf;
    /* find reqhdr */
    h->reqhdr = str_spilt(h->reqline, '\n', tmp->buf, tmp->size);
    if (h->reqhdr == NULL) {
        LOG("REQ LINE: not found\n");
        return h;
    }
    LOG("REQ LINE: %s\n", tmp->buf);

    /* parse reqline */
    (void)str_spilt(tmp->buf, ' ', h->methd, sizeof(h->methd));
    LOG("    method: %s\n", h->methd);

    /* parse reqhdr */
    const char *cur = h->reqhdr;
    for (;;) {
        buf_reset(tmp);
        cur = str_spilt(cur, '\n', tmp->buf, tmp->size);
        if (cur == NULL) {
            break;
        }
        LOG("REQ HDR : %s\n", tmp->buf);

        /* find reqbody */
        char hdrkey[MAX_HDR_KEY_LEN];
        (void)str_spilt(tmp->buf, '\n', hdrkey, sizeof(hdrkey));
        if (hdrkey[0] == '\r') {
            h->reqbody = cur;
            LOG("REQ BODY: %s\n", h->reqbody);
            break;
        }
    }

    return h;
}

void http_send_resmsg(KdBuf *sendbuf, int rescode, const char *reshdr[], const char *resbody)
{
    sprintf(sendbuf->buf, "HTTP/1.0 %d %s\r\n", rescode, rescode >= 300 ? "BAD REQUEST" : "OK");
    sendbuf->used += strlen(sendbuf->buf);

    int i;
    for (i = 0; reshdr[i] != NULL; i++) {
        buf_write_str(sendbuf, reshdr[i]);
        buf_write_str(sendbuf, "\r\n");
    }

    buf_write_str(sendbuf, "\r\n");
    buf_write_str(sendbuf, resbody);
    buf_write_str(sendbuf, "\r\n");

    LOG("RES MSG: %s\n<END>\n\n", sendbuf->buf);
}