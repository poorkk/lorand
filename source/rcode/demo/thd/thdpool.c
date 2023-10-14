#include "thd.h"

typedef struct {
    ThdCtrl *pool;
    int cnt;
} ThdPool;