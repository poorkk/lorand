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

#include "sock.h"

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

    return s;
}

KdSock *kd_sock_listen(KdSock *s)
{
    static bool islisten = false;
    int ret;

    if (!islisten) {
        ret = bind(s->sock, (struct sockaddr *)&s->addr, sizeof(s->addr));
        if (ret < 0) {
            printf("failed to bind %s %d, errcode: %d", s->ip, s->port, ret);
            ASSERT(false);
        }

        ret = listen(s->sock, 5);
        if (ret < 0) {
            ASSERT(false);
        }
        printf("listen: %s %d", s->ip, s->port);

        islisten = true;
    }

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
        printf("--[server] connect | ip: %s port: %d", c->ip, c->port);

        return c;
    }
}

void kd_sock_conn(KdSock *s)
{
    int ret;

    ret = connect(s->sock, (struct sockaddr *)&s->addr, sizeof(s->addr));
    if (ret < 0) {
        printf("error: failed to connect %s %d", s->ip, s->port);
        ASSERT(false);
    }

    printf("connect %s %d", s->ip, s->port);
}

void kd_sock_send(KdSock *s, const char *data, size_t datalen)
{
    int sendlen;
    sendlen = send(s->sock, data, datalen, 0);
    if (sendlen < 0) {
        printf("failed to send data, errcode: %d",  sendlen);
        ASSERT(false);
    }
}

int kd_sock_recv(KdSock *s, char *buf, size_t bufsz)
{
    int recvlen = 0;

    recvlen = recv(s->sock, buf, bufsz, 0);
    if (recvlen <= 0) {
        printf("--[%d] finish | ip: %s, port: %d", s->id, s->ip, s->port);
        return recvlen;
    } else {
        printf("--[%d] recv | len: %d, data: %s", s->id, recvlen, buf);
    }
    
    return recvlen;
}