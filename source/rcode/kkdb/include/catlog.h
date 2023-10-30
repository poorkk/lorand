#ifndef CATLOG_H_
#define CATLOG_H_

typedef struct {
    const char *name;
    int type;
} SysAttr;

typedef struct {
    const char *name;
    SysAttr attrs[];
} SysRel;


void catlog_test();

#endif
