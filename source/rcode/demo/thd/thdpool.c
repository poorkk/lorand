#include "thd.h"

typedef struct {
    ThdData *pool;
    int cnt;
} ThdPool;