#include <string.h>
#include "thd.h"
#include "sock.h"
#include "sess.h"
#include "http.h"

KdSess *kd_sess_new(KdSock *server, KdSock *client)
{
    KdSess *sess = (KdSess *)malloc(sizeof(KdSess));
    if (sess == NULL) {
        ASSERT(false);
        return NULL;
    }

    sess->server = server;
    sess->client = client;

    return sess;
}

void kd_cmd_sess_loop(KdThd *thd)
{
    if (thd == NULL) {
        ASSERT(false);
        return;
    }

    KdSess *sess = (KdSess *)thd->data;
    if (sess == NULL) {
        ASSERT(false);
        return;
    }

    KdSock *c = sess->client;
    int id = thd->id;
    for (;;) {
        /* receive client data */
        int recvlen;

        buf_reset(c->recvbuf);
        recvlen = recv(c->sock, c->recvbuf->buf, buf_freesz(c->recvbuf), 0); /* 一直read */
        if (recvlen > 0) {
            LOG("--[%d] recv | len: %d, data: %s\n", id, recvlen, c->recvbuf->buf);
            c->recvbuf->used += recvlen;
        } else if (recvlen == 0) { /* client finiish */
            LOG("--[%d] finish | ip: %s, port: %d\n", id, c->ip, c->port);
            break;
        } else { /* recvlen < 0 */
            printf("[%d] failed to receive, errcode: %d\n", id, recvlen);
            break;
        }

        /* send data */
        int sendlen;

        buf_reset(c->sendbuf);
        char tmp[1024];
        sprintf(tmp, "from server, sess id [%d], message [%s]", id, c->recvbuf->buf);
        buf_write_str(c->sendbuf, tmp);

        LOG("--[%d] send | len: %d, data: %s\n", id, strlen(c->sendbuf->buf), c->sendbuf->buf);
        sendlen = send(c->sock, c->sendbuf->buf, c->sendbuf->used, 0);
        ASSERT(sendlen >= 0);
    }
}

void kd_http_sess_loop(KdThd *thd)
{
    if (thd == NULL) {
        ASSERT(false);
        return;
    }

    KdSess *sess = (KdSess *)thd->data;
    if (sess == NULL) {
        ASSERT(false);
        return;
    }

    KdSock *c = sess->client;
    int id = thd->id;
    for (;;) {
        /* receive client data */
        int recvlen;

        buf_reset(c->recvbuf);
        recvlen = recv(c->sock, c->recvbuf->buf, buf_freesz(c->recvbuf), 0); /* 一直read */
        if (recvlen > 0) {
            LOG("--[%d] recv | len: %d, data: %s\n", id, recvlen, c->recvbuf->buf);
            c->recvbuf->used += recvlen;
        } else if (recvlen == 0) { /* client finiish */
            LOG("--[%d] finish | ip: %s, port: %d\n", id, c->ip, c->port);
            break;
        } else { /* recvlen < 0 */
            printf("[%d] failed to receive, errcode: %d\n", id, recvlen);
            break;
        }
        
        /* parse request */
        KdHttp *h = http_parse_reqmsg(c->recvbuf);

        /* send response */
        int sendlen;
        buf_reset(c->sendbuf);

        const char *hdr[10] = {0};
        hdr[0] = "Content-type: text/html\r\n";

        const char *body = "<HTML>"
            "<TITLE>kk home</TITLE>\r\n"
            "<BODY>"
            "<h1>li yan xuan</h1>"
            "<h2>shi</h2>"
            "<h1>hao bao bei</h1>"
            "</BODY>\r\n"
            "</HTML>";

        http_send_resmsg(c->sendbuf, 200, hdr, body);

        sendlen = send(c->sock, c->sendbuf->buf, c->sendbuf->used, 0);
        LOG("--[%d] send | len: %d, data: %s\n", id, strlen(c->sendbuf->buf), c->sendbuf->buf);
        ASSERT(sendlen >= 0);
    }
}

