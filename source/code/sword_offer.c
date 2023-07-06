#include <stdio.h>
#include <stdlib.h>

typedef char bool;
#define true 1
#define false 0

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)

bool tree_post_traverse(int *arr, int arrsz)
{
    if (arrsz <= 0) {
        return true;
    }

    int root;
    int left;
    int right;

    root = arrsz - 1;

    for (left = 0; left < root; left++) {
        if (arr[left] > arr[root]) {
            break;
        }
    }
    for (right = left; right < root; right++) {
        if (arr[right] < arr[root]) {
            break;
        }
    }
    if (right != root) {
        return false;
    }
    return tree_post_traverse(&arr[0], left - 0) && tree_post_traverse(&arr[left], root - left);
}