#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "str.h"
#include "cmd.h"

KdCmd *kd_cmd_init()
{
#define KD_INPUT_BUF_LEN 1024
    KdCmd *cmd = (KdCmd *)malloc(sizeof(KdCmd));

    cmd->cur_tsk = strdup("t0");
    cmd->buf = (char *)malloc(KD_INPUT_BUF_LEN);
    cmd->bufused = 0;
    cmd->bufsz = KD_INPUT_BUF_LEN;

    return cmd;
}

void kd_cmd_scan(KdCmd *cmd)
{
    cmd->buf[0] = '\0';
    cmd->bufused = 0;

    for (;;) {
        int scanlen;

        if (cmd->bufused + 256 >= cmd->bufsz) {
            break;
        }
        printf("%s < ", cmd->cur_tsk);
        gets(cmd->buf + cmd->bufused);
        cmd->bufused += strlen(cmd->buf + cmd->bufused);

        if (str_end_with(cmd->buf, cmd->bufused, ';', ' ')) {
            break;
        }
    }
}

void kd_cmd_prt(KdCmd *cmd)
{
    printf("%s > ", cmd->cur_tsk);
}

void kd_cmd_test()
{
    KdCmd *cmd = kd_cmd_init();

    for (;;) {
        kd_cmd_scan(cmd);

        kd_cmd_prt(cmd);
        printf("cmd %s\n", cmd->buf);
    }
}