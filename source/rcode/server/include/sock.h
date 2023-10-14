#ifndef KD_SOCK_H_
#define KD_SOCK_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * ref: https://www.cnblogs.com/huangliu1111/p/15351477.html
 */

typedef struct {
    int sock;
    /* server info */
    char *ip;
    int port;
    struct sockaddr_in addr;
    /* recv buf */
    char *recvbuf;
    int recvused;
    int recvsz;
    /* send buf */
    char *sendbuf;
    int sendused;
    int sendsz;
} KdServer;

typedef struct {
    int sock;
    /* client info */
    char *ip;
    int port;
    struct sockaddr_in addr;
    socklen_t addrsz;
    /* other info */
    char *name;
    /* recv buf */
    char *recvbuf;
    int recvused;
    int recvsz;
    /* send buf */
    char *sendbuf;
    int sendused;
    int sendsz;
} KdClient;

KdServer *kd_server_init(const char *ip, int port);
void kd_server_listen(KdServer *s);

KdClient *kd_client_init();
void kd_client_conn(KdClient *c, const char *ip, int port);
void kd_client_send(KdClient *c, const char *data, size_t datalen);
void kd_client_recv(KdClient *c);

#endif