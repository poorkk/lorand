#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "catlog.h"
#include "comm/str.h"

/* 
 * --------------------------- sys relations define ----------------------------------
 */

typedef char SysDataType;

#define SYS_END 0x00
#define SYS_CHAR 0x01
#define SYS_TEXT 0x03
#define SYS_INT 0x04
#define SYS_BYTE 0x05

typedef struct {
    const char *name;
    SysDataType types[40];
    const char *relation_vals[];
} SysRel;

/* 
 * --------------------------- sys relations ----------------------------------
 */

typedef struct {
    int rel_oid;
    const char *rel_name;
} SysClass;

SysRel sys_class = {
    "sys_class",
    {SYS_INT, SYS_TEXT, 0},
    {
        " 1 | sys_class ",
        " 2 | sys_attr ",
        /* do not add new relation by yourself, we will do it automatically */
    }
};

typedef struct {
    int rel_oid;
    const char *rel_name;
    int attr_idx;
    const char *attr_name;
    int attr_type;
} SysAttr;

SysRel sys_attr = {
    "sys_attr",
    {SYS_INT, SYS_TEXT, SYS_INT, SYS_TEXT, SYS_INT},
    {
        "1 | pg_class | 0 | rel_oid  | SYS_INT ",
        "1 | pg_class | 1 | rel_name | SYS_TEXT ",

        "2 | pg_attr  | 0 | rel_oid   | SYS_INT ",
        "2 | pg_attr  | 1 | rel_name  | SYS_TEXT ",
        "2 | pg_attr  | 2 | attr_idx  | SYS_INT ",
        "2 | pg_attr  | 3 | attr_name | SYS_TEXT ",
        "2 | pg_attr  | 4 | attr_type | SYS_INT ",

        /* do not add new attr by yourself, we will do it automatically */
    }
};

/* 
 * --------------------------- all sys relations ------------------------------
 */

SysRel *sys_rels[] = {
    /* manually update */
    &sys_class,
    &sys_attr,

    /* aotu update */
};


/* 
 * --------------------------- sys functions ----------------------------------
 */
typedef union  
{
    long ival;
    char *sval;
} Data;


typedef struct {
    int datacnt;
    SysDataType *datatype;
    Data *data
} TupData;

void form_tuple(TupData *tup, char *buf, size_t bufsz)
{
    int i;
    SysDataType type;
    int bufused = 0;
    int datalen;

    for (i = 0; i < tup->datacnt; i++) {
        type = tup->datatype[i];
        switch (type) {
            case SYS_CHAR:
                (buf + bufused)[bufused++] = tup->data[i].ival;
                break;
            case SYS_INT:
                sprintf(buf + bufused, "%d", tup->data[i].ival);
                bufused += sizeof(int);
                break;
            case SYS_TEXT:
                strcat(buf + bufused, tup->data[i].sval);
                bufused += (strlen(tup->data[i].sval) + 1);
                break;
            case SYS_BYTE:
                datalen = ((int *)tup->data[i].sval[0]);
                memcpy(buf + bufused, tup->data[i].sval, );
                bufused += (strlen(tup->data[i].sval) + 1);
                break;
            default:
                printf("invalid data type: %d\n", type);
                break;
        }
    }
}

void unform_tuple(char *buf, size_t bufsz, TupData *tup)
{
    int i;
    SysDataType type;
    int cur;

    for (i = 0; i < tup->datacnt; i++) {
        type = tup->datatype[i];
        Data *data = &tup->data[i];
        switch (type) {
            case SYS_CHAR:
                data->ival = (buf + cur)[0];
                cur += sizeof(char);
                break;
            case SYS_INT:
                data->ival = ((int *)(buf + cur))[0];
                cur += sizeof(int);
                break;
            case SYS_TEXT:
                data->sval = buf + cur;
                cur += strlen(data->sval) + 1;
                break;
            case SYS_BYTE:
                data->sval = buf + cur;
                cur += ((int *)data->sval)[0];
                break;
            default:
                printf("invalid data type: %d\n", type);
                break;
        }
    }
}

typedef struct {
    char *sys_dir;
    char *path;

    int fd_class;
    int fd_attr;
} SysInitMgr;

void sysrel_init(SysInitMgr *mgr, SysRel *rel)
{
    
}

void sysrel_init_all(SysInitMgr *mgr, SysRel *sys_rels[])
{
    /* init 'sys_class' at first */

    /* init 'sys_attr' */

    /* init other sys relation */
}

void catlog_test()
{
;
}
