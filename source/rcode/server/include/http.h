#ifndef KD_HTTP_H_
#define KD_HTTP_H_

#include "str.h"

#define HTTP_METHD_BUF_SZ 20

typedef struct {
    KdBuf *kbuf;

    const char *reqline;
    char methd[HTTP_METHD_BUF_SZ];

    const char *reqhdr;
    const char *reqbody;

    KdBuf *tmp;
} KdHttp;

KdHttp *http_parse_reqmsg(KdBuf *buf);
void http_send_resmsg(KdBuf *sendbuf, int rescode, const char *reshdr[], const char *resbody);
#endif