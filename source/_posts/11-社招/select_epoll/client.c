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

int myid = 0;

void do_connect()
{
    int id = myid;
    for (int i = 0; i < 100; i++) {
        printf("%d\n", id);
        sleep(1);
    }    
}

int main()
{
    int i;

    for (i = 0; i < 100; i++) {
        myid = i;
        pid_t pid = fork();
        if (pid == 0) {
            do_connect();
        }
        sleep(2);
    }

    return 0;
}