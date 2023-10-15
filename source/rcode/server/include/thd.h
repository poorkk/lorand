#ifndef KD_THD_H_
#define KD_THD_H_

#include <pthread.h>

typedef struct KdThd KdThd;

typedef void (*FuncThdRun)(KdThd *arg);

struct KdThd {
    int stat;
    pthread_t thd;
    int id;

    FuncThdRun func_run;
    void *data;
};

KdThd *thd_new(FuncThdRun func, void *data);
void test_thd();

#endif
