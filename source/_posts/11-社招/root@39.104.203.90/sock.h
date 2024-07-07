#ifndef KD_SOCK_H_
#define KD_SOCK_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdlib.h>

#define bool char
#define true 1
#define false 0

/* error */
#define ASSERT(cond) \
do { \
    if (!(cond)) { \
        printf("ASSERT | file: %s  line: %d  func: %s\n", __FILE__, __LINE__, __FUNCTION__); \
        exit(-1); \
    } \
} while (0)

#define STACK printf("-- [STACK] -- %-20s  %d\n", __FUNCTION__, __LINE__);

typedef struct {
    char *buf;
    size_t used;
} Err;

#define errmsg(err, fmt, ...) sprintf((err)->buf, (fmt), ##__VA_ARGS__)

Err *err_new(size_t bufsz);

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

    int id;
} KdSock;

KdSock *kd_sock_new(const char *ip, int port);
KdSock *kd_sock_init(int sock, struct sockaddr_in *addr);

KdSock *kd_sock_listen(KdSock *s);
void kd_sock_conn(KdSock *s);

void kd_sock_send(KdSock *s, const char *data, size_t datalen);
int kd_sock_recv(KdSock *s, char *buf, size_t bufsz);

#endif