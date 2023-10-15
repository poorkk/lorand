#include <stdio.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "sock.h"
#include "str.h"
#include "http.h"
#include "thd.h"
#include "sess.h"

KdSock *kd_sock_new(const char *ip, int port)
{
    KdSock *s;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(port), {0}};
    inet_aton(ip, &addr.sin_addr);

    s = kd_sock_init(sock, &addr);

    ASSERT(s->sock >= 0);
    ASSERT(ip != NULL);
    ASSERT(s->recvbuf != NULL);
    ASSERT(s->sendbuf != NULL);

    return s;
}

KdSock *kd_sock_init(int sock, struct sockaddr_in *addr)
{
    KdSock *s;
    
    s = (KdSock *)malloc(sizeof(KdSock));
    if (s == NULL) {
        return NULL;
    }

    s->sock = sock;
    memcpy(&s->addr, addr, sizeof(s->addr));

    s->ip = strdup(inet_ntoa(addr->sin_addr));
    s->port = ntohs(addr->sin_port);

    s->recvbuf = buf_malloc(MAX_SOCK_BUF_SZ);
    s->sendbuf = buf_malloc(MAX_SOCK_BUF_SZ);

    return s;
}

void kd_sock_listen(KdSock *s)
{
    int ret;

    ret = bind(s->sock, (struct sockaddr *)&s->addr, sizeof(s->addr));
    if (ret < 0) {
        LOG("failed to bind %s %d, errcode: %d\n", s->ip, s->port, ret);
        ASSERT(false);
    }

    ret = listen(s->sock, 5);
    if (ret < 0) {
        ASSERT(false);
    }
    LOG("listen: %s %d\n", s->ip, s->port);

    for (;;)  {
        /* client info */
        int csock;
        struct sockaddr_in caddr = {0};
        socklen_t caddrsz;
        KdSock *c;

        csock = accept(s->sock, (struct sockaddr *)&caddr, &caddrsz);
        ASSERT(csock >= 0);

        c = kd_sock_init(csock, &caddr);
        ASSERT(c != NULL);
        LOG("--[server] connect | ip: %s port: %d\n", c->ip, c->port);

        KdThd *sessthd = thd_new(kd_http_sess_loop, kd_sess_new(s, c));
    }
}

void kd_sock_conn(KdSock *s)
{
    int ret;

    ret = connect(s->sock, (struct sockaddr *)&s->addr, sizeof(s->addr));
    if (ret < 0) {
        printf("error: failed to connect %s %d\n", s->ip, s->port);
        ASSERT(false);
    }

    LOG("connect %s %d\n", s->ip, s->port);
}

void kd_sock_send(KdSock *s, const char *data, size_t datalen)
{
    int sendlen;
    sendlen = send(s->sock, data, datalen, 0);
    if (sendlen < 0) {
        printf("failed to send data, errcode: %d\n",  sendlen);
        ASSERT(false);
    }
}

void kd_sock_recv(KdSock *s)
{
    int recvlen;

    buf_reset(s->recvbuf);

    recvlen = recv(s->sock, s->recvbuf->buf, buf_freesz(s->recvbuf), 0);
    if (recvlen < 0) {
        printf("failed to receive, errcode: %d\n",  recvlen);
    }
    s->recvbuf->used += recvlen;
}

#ifndef KD_SSL
// https://blog.csdn.net/sardden/article/details/42705897
void ssl_server_conn(KdSock *sock, const char *cafile, const char *ser_priv)
{
    int ret;
    
    /* init */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    /* load */
    sock->ctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX *ctx = sock->ctx;
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }
    ret = SSL_CTX_use_certificate_file(ctx, cafile, SSL_FILETYPE_PEM);
    if (ret <= 0) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }
    ret = SSL_CTX_use_PrivateKey_file(ctx, ser_priv, SSL_FILETYPE_PEM);
    if (ret =  0) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }
    ret = SSL_CTX_check_private_key(ctx);
    if (ret =  0) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }

    sock->ssl = SSL_new(ctx);
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }
    SSL_set_fd(sock->ssl, sock);
    /* ssl conn */
    ret = SSL_accept(sock->ssl);
    if (ret <= 0) {
        ERR_print_errors_fp(stdout);
        ASSERT(false);
    }
}
#endif