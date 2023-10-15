#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int fdarr[10];
    int fdcnt;
    char *bufarr[10];
} ConnPool;

ConnPool *ConnPoolInit()
{
    ConnPool *pool = (ConnPool *)malloc(sizeof(ConnPool));
    memset(pool, 0, sizeof(ConnPool));
    for (int i = 0; i < 10; i++) {
        pool->bufarr[i] = (char *)malloc(200);
    }
    return pool;
}

void ConnPoolAdd(ConnPool *pool, int fd)
{
    pool->fdarr[pool->fdcnt++] = fd;
}

void ConnPoolPoll(ConnPool *pool)
{
    for (int i = 0; i < pool->fdcnt; i++) {
        int curfd = pool->fdarr[i];
        if (curfd <= 0) {
            continue;
        }
        int readsz = read(curfd, pool->bufarr[i], 200);
        if (readsz > 0) {
            printf("readsz: %d | %s\n", readsz, pool->bufarr[i]);
        }

    }
}

void ProcessMain(int cfd)
{
    char readbuf[4096] = {0};
    for (;;) {
        int readsz = read(cfd, &readbuf, sizeof(readbuf)); /* 一直read */
        if (readsz > 0) {
            printf("readsz: %d | %s\n", readsz, readbuf);
        }

        close(cfd);
        printf("close\n");
        break;
    }
}

int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in saddr = {AF_INET, htons(8888), {0}};
    inet_aton("127.0.0.1", &saddr.sin_addr);

    bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));

    listen(sfd, 5);

    printf("listen\n");

    struct sockaddr_in caddr = {0};
    socklen_t caddrsz = 0;
    for (;;)  {
        int cfd = accept(sfd, (struct sockaddr *)&caddr, &caddrsz);

        printf("accept: %s\n", inet_ntoa(caddr.sin_addr));

        if (fork() == 0 ) {
            ProcessMain(cfd);
            exit(0);
        }
    }

    return 0;
}