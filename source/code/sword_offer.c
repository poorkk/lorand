#include <stdio.h>
#include <stdlib.h>

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)

void tree_post_travl(int *arr, int arrsz)
{
    
}