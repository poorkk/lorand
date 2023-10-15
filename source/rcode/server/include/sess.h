#ifndef KD_SESS_H_
#define KD_SESS_H_

#include "thd.h"
#include "sock.h"

typedef struct {
    int type;
    KdSock *server;
    KdSock *client;
} KdSess;

KdSess *kd_sess_new(KdSock *server, KdSock *client);
void kd_cmd_sess_loop(KdThd *thd);
void kd_http_sess_loop(KdThd *thd);

#endif
