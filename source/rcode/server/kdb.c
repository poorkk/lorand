

#include <stdio.h>
#include <string.h>
#include "str.h"
#include "sock.h"
#include "cmd.h"
#include "thd.h"
#include "file.h"

#define STR(s) #s

#define HOST "192.168.3.23"

void kd_help()
{
    const char *help = ""
        "usage:  \n"
        "    kdb server PORT\n"
        "    kdb client PORT\n"
        "";

    printf("[%s]\n", help);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        kd_help();
        return 0;
    }

    if (strcmp(argv[1], "server") == 0) {
        printf(" http://%s:%d\n", HOST, atoi(argv[2]));
        KdSock *s = kd_sock_new(HOST, atoi(argv[2]));

        kd_sock_listen(s);

        return 0;
    }

    if (strcmp(argv[1], "client") == 0) {
        KdCmd *cmd = kd_cmd_init();
        ASSERT(cmd != NULL);

        KdSock *c = kd_sock_new(HOST, atoi(argv[2]));
        ASSERT(c != NULL);

        kd_sock_conn(c);

        for (;;) {
            kd_cmd_scan(cmd);

            kd_sock_send(c, cmd->buf, cmd->bufused);
            kd_sock_recv(c);

            kd_cmd_prt(cmd);
            printf("%s\n", c->recvbuf->buf);
        }

        return 0;
    }

    printf("error: %s\n", argv[1]);

    return 0;
}