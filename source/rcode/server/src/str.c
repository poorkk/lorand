#include "str.h"
#include <stdio.h>

/*
 * buf: xxxeaaa
 * end: e, skip: a
 * return true
 */
bool str_end_with(const char *buf, int buflen, char end, char skip)
{
    if (buflen == 0) {
        return false;
    }

    for (int i = buflen - 1; i >= 0; i--) {
        if (buf[i] == skip) {
            continue;
        }
        if (buf[i] == end) {
            return true;
        }
        return false;
    }
    return false;
}