#ifndef THD_H_
#define THD_H_

#include <pthread.h>

typedef enum {
    THD_ERR = 0,
    THD_INIT,
    THD_ACTIVE,
    THD_END
} ThdStatus;

typedef void (*ThdFunc)(void *arg);

typedef struct {
    pthread_t thd;

    /* thd ctrl */
    pthread_mutex_t run;

    /* thd info */
    ThdStatus status;
    int id;
    const char *name;

    ThdFunc hookfunc;
    void *arg;
} ThdData;

#endif