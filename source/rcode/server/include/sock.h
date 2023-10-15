#ifndef KD_SOCK_H_
#define KD_SOCK_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#ifndef KD_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include "str.h"

/*
 * ref: https://www.cnblogs.com/huangliu1111/p/15351477.html
 */

#define MAX_SOCK_BUF_SZ 4096

typedef struct {
    int sock;
    /* server info */
    char *ip;
    int port;
    struct sockaddr_in addr;
    /* data buffer */
    KdBuf *recvbuf;
    KdBuf *sendbuf;
#ifndef KD_SSL
    /* ssl  */
    bool usessl;
    SSL *ssl;
    SSL_CTX *ctx;
#endif
} KdSock;

KdSock *kd_sock_new(const char *ip, int port);
KdSock *kd_sock_init(int sock, struct sockaddr_in *addr);

void kd_sock_listen(KdSock *s);
void kd_sock_conn(KdSock *s);
void kd_sock_send(KdSock *s, const char *data, size_t datalen);
void kd_sock_recv(KdSock *s);

#endif