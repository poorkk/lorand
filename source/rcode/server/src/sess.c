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
        recvlen = kd_sock_recv(c);
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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

typedef enum {
    KD_ERRO = 0,
    KD_CONTINUE,
    KD_FINISH
} KdRunStatus;

KdRunStatus http_send_index(KdSock *c, KdHttp *h)
{
    KdBuf *sendbuf = c->sendbuf;
    
    buf_reset(sendbuf);

    const char *hdr[10] = {0};
    hdr[0] = "Content-type: text/html\r\n";
    http_send_resmsg(sendbuf, 200, hdr, NULL);

    sendbuf->used += kd_file_read_all("./src/http/index.html", sendbuf->buf + sendbuf->used, buf_freesz(sendbuf));

    kd_sock_flush_send(c);

    close(c->sock);

    return KD_FINISH;
}

KdRunStatus http_receive_file(KdSock *c, KdHttp *h)
{
    char *start;
    char *end;
    KdBuf *rebuf = c->recvbuf;
    int recvlen = rebuf->used;
    int fdnew = -1;
    bool succeed = false;
    int id = c->id;

    do {
        /* find boundary */
#define BOUNDARY "boundary="
        start = strstr(h->reqhdr, BOUNDARY);
        if (start == NULL) {
            printf("failed to find boundary from header\n");
            break;
        }
        start += strlen(BOUNDARY);
        for (end = start; end[0] != '\0'; end++) {
            if (end[0] == ' ' || end[0] == '\r' || end[0] == '\n') {
                break;
            }
        }
        end[0] = '\0';
        LOG("boundary [%s]\n", start);
        char *bound = strdup(start);

        /* find file name */
#define RECV_FILE_NAME "filename=\"" 
        start = strstr(h->reqbody, bound);
        if (start == NULL) {
            printf("failed to find find boundary from request body\n");
            break;
        }
        start = strstr(start, RECV_FILE_NAME);
        if (start == NULL) {
            printf("failed to find file name\n");
            break;
        }
        start += strlen(RECV_FILE_NAME);
        for (end = start; end[0] != '\0' && end[0] != '"'; end++) {};
        end[0] = '\0';
        LOG("--[%d] recv file | %s\n", id, start);

        /* generate file */
        fdnew = open(start, O_CREAT | O_RDWR, 0600);
        if (fdnew < 0) {
            printf("failed to generate file '%s'\n", start);
        }

        /* find content start */
#define CONTENT_START "\r\n\r\n"
        start = strstr(end + 1, CONTENT_START);
        if  (start == NULL) {
            printf("failed to file start of '/r/n/r/n'\n");
            break;
        }
        start += strlen(CONTENT_START); /* start */
        
        /* find content end */
        end = kd_strstr(start, recvlen - (start - rebuf->buf), bound);
        if (end != NULL) { /* end in current message */
            end -= strlen("\r\n--");
            LOG("<%d>\n", end - start);
            write(fdnew, start, end - start);
        } else {
            write(fdnew, start, recvlen - (start - rebuf->buf));
            /* keep receiving message */
            for (;;) {
                recvlen = kd_sock_recv(c);
                if (recv <= 0) {
                    break;
                }
                end = kd_strstr(rebuf->buf, recvlen, bound);
                if (end != NULL) {
                    end -= strlen("\r\n--");
                    LOG("<%d>\n", end - rebuf->buf);
                    write(fdnew, rebuf->buf, end - rebuf->buf);
                    break;
                } else {
                    LOG("%d\n", recvlen);
                    write(fdnew, rebuf->buf, recvlen);
                }
            }
        }

        LOG("--[%d] recv file finish\n", id);
        http_send_index(c, h);
        succeed = true;
    } while (0);

    if (fdnew != -1) {
        close(fdnew);
    }

    close(c->sock);

    return KD_FINISH;
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
    c->id = thd->id;

    for (;;) {
        /* receive client data */
        int recvlen;

        buf_reset(c->recvbuf);
        recvlen = kd_sock_recv(c); /* 一直read */
        if (recvlen <= 0) { /* client finiish */    
            break;
        }
        
        /* parse request */
        KdHttp *h = http_parse_reqmsg(c->recvbuf);

        /* response build */
        if (strcasecmp(h->methd, "GET") == 0) {
            http_send_index(c, h);
            break;
        } else if (strcasecmp(h->methd, "POST") == 0) {
            if (strcasecmp(h->uri, "/recvfile") == 0) { /* receive file */
                http_receive_file(c, h);
                break;
            }
        }
    }
}