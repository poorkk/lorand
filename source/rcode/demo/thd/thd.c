#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "thd.h"

/* used by thd self */
#define THD_RUN_READY(tdata) pthread_mutex_lock(&((ThdData *)(tdata))->run)
#define THD_RUN_END(tdata) pthread_mutex_unlock(&((ThdData *)(tdata))->run)
/* used by thd controler */
#define THD_SET_WAIT THD_RUN_READY
#define THD_SET_RUN THD_RUN_END

/* thread mainloop */
void *thd_loop(void *arg)
{
    ThdData *tdata = (ThdData *)arg;

    for (;;) {
        THD_RUN_READY(tdata);

        printf("%d\n", tdata->id);

        if (tdata->hookfunc != NULL) {
            tdata->hookfunc(arg);
        }

        THD_RUN_END(tdata);
    }
    return NULL;
}

/* create thread */
void thd_init(ThdData *tdata)
{
    int status;
    
    pthread_mutex_init(&tdata->run, NULL);
    THD_SET_WAIT(tdata);

    tdata->status = THD_INIT;
    tdata->id = 0;
    tdata->name = NULL;

    tdata->hookfunc = NULL;
    tdata->arg = NULL;

    status = pthread_create(&tdata->thd, NULL, thd_loop, tdata);
    if (status != 0) {
        tdata->status = THD_ERR;
    }
}

void thd_add(ThdData *tdata, ThdFunc func)
{
    tdata->hookfunc = func;
    THD_SET_RUN(tdata);
    printf("set run\n");
    THD_SET_WAIT(tdata);
}

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t hasdata;
    pthread_cond_t full;
} ThdCtrl;

int main()
{
    ThdData t;

    thd_init(&t);
    printf("1\n");
    fflush(stdout);

    sleep(1);

    thd_add(&t, NULL);

    printf("hello\n");

    sleep(1);

    pthread_join(t.thd, NULL);

    return 0;
}