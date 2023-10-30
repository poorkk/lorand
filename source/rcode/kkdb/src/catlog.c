#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "catlog.h"

/* 
 * --------------------------- sys relations ----------------------------------
 */

typedef struct {
    char type;
    union data
    {
        double ival;
        char *str;
    };
} SysType;

typedef struct {
    SysType oid;
    SysType name;
} KdRel;

const SysRel class = {
    "class",
    {
        {"oid", 1},
        {"relname", 2},
        {"attrcnt", 1},
        {0}
    }
};

const char *initvals[] = {
    " 1 | t1 | 5",
    " 2 | t2 | 9"
};

/* 
 * --------------------------- all sys relations ------------------------------
 */

SysRel *sysrels[] = {
    &class,
};

/* 
 * --------------------------- sys functions ----------------------------------
 */

void sysrel_init(SysRel *rel, char **initval[])
{
    int i;
    for (i = 0; initval[i] != NULL; i++) {
        printf("%s\n", initval[i]);
    }
}

void sysrel_init_all()
{
    ;
}

void catlog_test()
{
    int i = 0;
    for (i = 0; class.attrs[i].name != NULL; i++) {
        printf("%s\n", class.attrs[i].name);
    }
    printf("%d\n", i);
    sysrel_init(&class, NULL);
}
