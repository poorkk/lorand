#ifndef KD_CMD_H_
#define KD_CMD_H_

typedef struct {
    char *cur_tsk;
    char *buf;
    int bufused;
    int bufsz;
} KdCmd;

KdCmd *kd_cmd_init();
void kd_cmd_scan(KdCmd *cmd);
void kd_cmd_prt(KdCmd *cmd);

void kd_cmd_test();

#endif