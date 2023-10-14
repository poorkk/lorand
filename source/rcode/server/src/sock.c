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

KdServer *kd_server_init(const char *ip, int port)
{
    KdServer *s = (KdServer *)malloc(sizeof(KdServer));

    s->sock = socket(AF_INET, SOCK_STREAM, 0);

    s->ip = strdup(ip);
    s->port = port;

    struct sockaddr_in addr = {AF_INET, htons(s->port), {0}};
    inet_aton(s->ip, &s->addr.sin_addr);

    return s;
}

void kd_server_listen(KdServer *s)
{
    int ret;

    ret = bind(s->sock, (struct sockaddr *)&s->addr, sizeof(s->addr));
    if (ret < 0) {
        return;
    }

    ret = listen(s->sock, 5);
    if (ret < 0) {
        return;
    }

    LOG("listen: %s %d\n", s->ip, s->port);

    for (;;)  {
        KdClient *c = (KdClient *)malloc(sizeof(KdClient));
        ASSERT(c != NULL);

        c->sock = accept(s->sock, (struct sockaddr *)&c->addr, &c->addrsz);
        ASSERT(c >= 0);

        c->ip = inet_ntoa(c->addr.sin_addr);
        c->port = ntohl(c->addr.sin_port);
        LOG("recv: %s %d\n", c->ip, c->port);

        c->recvbuf = (char *)malloc(4096);
        ASSERT(c->recvbuf != NULL);
        c->recvsz = 4096;
        c->recvused = 0;
        memset(c->recvbuf, 0, c->recvsz);

        c->sendbuf = (char *)malloc(4096);
        ASSERT(c->sendbuf != NULL);
        c->sendsz = 4096;
        c->sendused = 0;
        memset(c->sendbuf, 0, c->sendsz);

        for (;;) {
            int recvlen = read(c->sock, c->recvbuf, c->recvsz - c->recvused); /* 一直read */
            ASSERT(recvlen > 0);

            LOG("recvlen: %d | %s\n", recvlen, c->recvbuf);

            c->recvused += recvlen;
            c->recvbuf[c->recvused] = '\0';

            strcat(c->sendbuf, "server: receive [");
            strcat(c->sendbuf, c->recvbuf);
            strcat(c->sendbuf, "]");

            int sendlen = write(c->sock, c->sendbuf, strlen(c->sendbuf));

            c->recvused = 0;
            memset(c->recvbuf, 0, c->recvused);
            memset(c->sendbuf, 0, c->sendsz);
        }
    }
}

KdClient *kd_client_init()
{
    KdClient *c = (KdClient *)malloc(sizeof(KdClient));
    ASSERT(c != NULL);

    c->sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(c->sock >= 0);

    // c->recvbuf = (char *)malloc(4096);
    // ASSERT(c->recvbuf != NULL);
    // c->recvsz = 4096;
    // c->recvused = 0;
    // memset(c->recvbuf, 0, c->recvsz);

    // c->sendbuf = (char *)malloc(4096);
    // ASSERT(c->sendbuf != NULL);
    // c->sendsz = 4096;
    // c->sendused = 0;
    // memset(c->sendbuf, 0, c->sendsz);

    return c;
}

void kd_client_conn(KdClient *c, const char *ip, int port)
{
    int ret;

    c->ip = strdup(ip);
    c->port = port;
    
    struct sockaddr_in addr = {AF_INET, htons(port), {0}};
    inet_aton(ip, &addr.sin_addr);
    memcpy(&c->addr, &addr, sizeof(c->addr));

    ret = connect(c, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        printf("error: failed to connect %s %d\n", ip, port);
        ASSERT(false);
    }

    LOG("connect %s %d\n", ip, port);
}

void kd_client_send(KdClient *c, const char *data, size_t datalen)
{
    int sendlen;
    LOG("send %s %lu\n", data, datalen);
    sendlen = write(c->sock, data, datalen);
    if (sendlen < 0) {
        ;
    }
}

void kd_client_recv(KdClient *c)
{
    int recvlen;

    memset(c->recvbuf, 0, c->recvused);

    recvlen = read(c->sock, c->recvbuf, c->recvsz);
    LOG("recv %s %lu\n", c->recvbuf, recvlen);
    c->recvused = recvlen;
}