#include <stdio.h>

#include "sock.h"

int main() 
{
    KdSock *server;
    KdSock *client;

    server = kd_sock_new("127.0.0.1", 19800);
    if (server == NULL) {
        return 0;
    }

    printf("dbserver is listen on (%s, %d)...", "127.0.0.1", 19800);

    for (;;) {
        client = kd_sock_listen(server);
        if (client == NULL) {
            break;
        }

        printf(" + new client: (%s, %d)", client->ip, client->port);
    }

    return 0;
}