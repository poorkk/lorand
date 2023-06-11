#include <stdio.h>

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)
void prt(int *arr, int arrlen);

/* 1 冒泡排序 */
void sort_bubble(int *arr, int arrlen)
{
    int alen;
    int i;

    for (alen = arrlen; alen > 1; alen--) {
        for (i = 0; i < alen - 1; i++) {
            if (arr[i] > arr[i + 1]) {
                SWAP(arr, i, i+1);
            }
        }
    }
}

/* 2 快速排序 */
void sort_quick(int *arr, int left, int right)
{
    int pivot;
    int le;
    int ri;
    
    le = left;
    ri = right;
    pivot = arr[le];
    while (le < ri) {
        while (le < ri && arr[ri] >= pivot) {
            ri--;
        }
        arr[le] = arr[ri];
        while (le < ri && arr[le] <= pivot) {
            le++;
        }
        arr[ri] = arr[le];
    }
    arr[le] = pivot;

    if (le < right) {
        sort_quick(arr, left, le - 1);
        sort_quick(arr, le + 1, right);
    }
}

/* 归并排序 */
void sort_merge(int *arr, int left, int right)
{
    // int mid = (right - left) / 2;
    // arr[right - left];
}

/* 辅助函数 */
void prt(int *arr, int arrlen)
{
    int i;
    for (i = 0; i < arrlen; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main()
{
    int arr[100] = {5, 9, 7, 10, 3, 1};
    sort_quick(arr, 0, 5);
    prt(arr, 6);

    return 0;
}