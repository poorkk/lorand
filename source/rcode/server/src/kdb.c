

#include <stdio.h>
#include <string.h>
#include "str.h"
#include "sock.h"
#include "cmd.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return 0;
    }
    
    if (strcmp(argv[1], "server") == 0) {
        KdServer *s = kd_server_init("192.168.3.11", 8888);

        kd_server_listen(s);
    }

    if (strcmp(argv[1], "client") == 0) {
        //KdCmd *cmd = kd_cmd_init();
        //ASSERT(cmd != NULL);

        KdClient *c = kd_client_init();
        ASSERT(c != NULL);

        kd_client_conn(c, "192.168.3.11", 8888);

        for (;;) {
            kd_cmd_scan(cmd);

            LOG("scan: %s | %d\n", cmd->buf, cmd->bufused);

            kd_cmd_prt(cmd);

            kd_client_send(c, cmd->buf, cmd->bufused);
            kd_client_recv(c);

            printf("%s\n", c->recvbuf);
        }
    }

    return 0;
}