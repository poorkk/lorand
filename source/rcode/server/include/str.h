#ifndef KD_STR_H_
#define KD_STR_H_

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond) \
do { \
    if (!(cond)) { \
        printf("error\n");\
        exit(-1); \
    } \
} while (0)
//printf("error:  file: %s  line: %s  func: %s\n", __FILE__, __LINE__, __FUNCTION__); 

#define LOG printf

#define bool char
#define true 1
#define false 0

bool str_end_with(const char *buf, int buflen, char end, char skip);

#endif