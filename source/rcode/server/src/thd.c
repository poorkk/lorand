#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "str.h"
#include "thd.h"

void *thd_main(void *arg)
{
    KdThd *thd = (KdThd *)arg;
    thd->func_run(thd);
    return NULL;
}

KdThd *thd_new(FuncThdRun func, void *data)
{
    static int thdid = 0;

    KdThd *t = (KdThd *)malloc(sizeof(KdThd));
    if (t == NULL) {
        ASSERT(false);
        return NULL;
    }

    t->stat = -1;
    t->id = thdid++;

    t->func_run = func;
    t->data = data;

    t->stat = pthread_create(&t->thd, NULL, thd_main, t);

    return t;
}

void test_run(KdThd *thd)
{
    printf("-> exec thread | id: %d\n", thd->id);
}
void test_thd()
{
    printf("test thd\n");
    KdThd *t = thd_new(test_run, NULL);
    pthread_join(t->thd, NULL);
}