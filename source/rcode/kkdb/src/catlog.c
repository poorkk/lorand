#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *name;
    int type;
} Attr;

typedef struct {
    const char *name;
    Attr attrs[];
} Relation;

Relation class = {
    "class",
    {
        {"oid", 1},
        {"relname", 2},
        {"attrcnt", 1},
        {0}
    }
};

void catlog_test()
{
    int i = 0;
    for (int i = 0; class.attrs[i].name != NULL; i++) {
        printf("%s\n", class.attrs[i].name);
    }
    printf("%d\n", i);
}
