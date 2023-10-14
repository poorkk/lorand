#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "thd.h"

typedef enum {
    THD_ERR = 0,
    THD_INIT,
    THD_ACTIVE,
    THD_END
} ThdStatus;

typedef void (*ThdTaskFunc)(void *arg);

typedef struct ThdTask {
    Node *next;
    int id;
    ThdTaskFunc func;
    void *arg;
} ThdTask;

typedef struct {
    pthread_t thd;

    /* thd ctrl */
    pthread_mutex_t run;

    /* thd info */
    ThdStatus status;
    int id;
    const char *name;

    ThdTask *taskqueue;
} ThdCtrl;

/* used by thd self */
#define THD_RUN_READY(tctl) pthread_mutex_lock(&(tctl)->run)
#define THD_RUN_END(tctl) pthread_mutex_unlock(&(tctl)->run)
/* used by thd controler */
#define THD_SET_WAIT THD_RUN_READY
#define THD_SET_RUN THD_RUN_END

/* thread mainloop */
void *thd_loop(void *thdctrl)
{
    ThdCtrl *tctl = (ThdCtrl *)thdctrl;

    for (;;) {
        THD_RUN_READY(tctl);

        printf("%d\n", tctl->id);

        if (tctl->hookfunc != NULL) {
            tctl->hookfunc(arg);
        }

        THD_RUN_END(tctl);
    }
    return NULL;
}

/* create thread */
void thd_init(ThdCtrl *tctl)
{
    int status;
    
    pthread_mutex_init(&tctl->run, NULL);
    THD_SET_WAIT(tctl);

    tctl->status = THD_INIT;
    tctl->id = 0;
    tctl->name = NULL;

    tctl->hookfunc = NULL;
    tctl->arg = NULL;

    status = pthread_create(&tctl->thd, NULL, thd_loop, tctl);
    if (status != 0) {
        tctl->status = THD_ERR;
    }
}

void thd_add(ThdCtrl *tctl, ThdTaskFunc func)
{
    tctl->hookfunc = func;
    THD_SET_RUN(tctl);
    printf("set run\n");
    THD_SET_WAIT(tctl);
}

// typedef struct {
//     pthread_mutex_t lock;
//     pthread_cond_t hasdata;
//     pthread_cond_t full;
// } ThdCtrl;

int main()
{
    ThdCtrl t;

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