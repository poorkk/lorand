

#include <stdio.h>
#include <string.h>
#include "str.h"
#include "sock.h"
#include "cmd.h"
#include "thd.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return 0;
    }

    if (strcmp(argv[1], "server") == 0) {
        KdSock *s = kd_sock_new("192.168.3.23", 8888);

        kd_sock_listen(s);

        return 0;
    }

    if (strcmp(argv[1], "client") == 0) {
        KdCmd *cmd = kd_cmd_init();
        ASSERT(cmd != NULL);

        KdSock *c = kd_sock_new("192.168.3.23", 7778);
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