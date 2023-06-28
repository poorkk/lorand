#include <stdio.h>
#include <string.h>

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)
#define ARRLEN(arr) (int)(sizeof((arr))/sizeof((arr)[0]))
void prt(int *arr, int arrlen)
{
    int i;
    for (i = 0; i < arrlen; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void move_zero(int *arr, int arrsz) // 冒泡排序
{
    int tmp;
    int i;

    if (arrsz <= 1) {
        return;
    }

    for (i = 0; i < arrsz - 1; i++) {
        if (arr[i] == 0) {
            tmp = arr[i];
            arr[i] = arr[i + 1];
            arr[i + 1] = tmp;
        }
    }
    arrsz--;
    move_zero(arr, arrsz);
}

int remove_num(int *arr, int arrsz, int num) // 双指针，冒泡排序
{
    int left;
    int right;
    int cnt = 0;

    if (arrsz < 1) {
        return 0;
    }

    for (left = 0, right = arrsz - 1; left <= right; left++) {
        if (arr[left] == num) {
            cnt++;
            for (; arr[right] == num && right > left; cnt++, right--){}
            if (left < right) {
                SWAP(arr, left, right);
                right--;
            }
        }
    }
    return arrsz - cnt;
}

int remove_dumpicate(int *arr, int arrsz) // 双指针
{
    int start;
    int end;
    int newsz = 0;

    if (arrsz <= 1) {
        return arrsz;
    }

    start = 0;
    end = start;
    while (start < arrsz && end < arrsz) {
        while (end < arrsz && arr[end] == arr[start]) {
            end++;
        }
        arr[newsz++] = arr[start];
        start = end;
    }

    return newsz;
}

int remove_dumpicate2(int *arr, int arrsz) // 双指针
{
    int start;
    int end;
    int newsz = 0;

    if (arrsz <= 1) {
        return arrsz;
    }

    start = 0;
    end = start;
    while (start < arrsz && end < arrsz) {
        while (end < arrsz && arr[end] == arr[start]) {
            end++;
        }
        arr[newsz++] = arr[start];
        if (end - start >= 2) {
            arr[newsz++] = arr[start];
        }
        start = end;
    }

    return newsz;
}

void bubble_sort(int *arr, int arrsz) // 冒泡排序
{
    if (arrsz <= 1) {
        return;
    }
    
    int i;
    for (i = 0; i < arrsz - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            SWAP(arr, i, i + 1);
        }
    }
    bubble_sort(arr, arrsz - 1);
}

int main()
{
    // int *arr = nums;
    // int arrsz = numsSize;
    int arr[] = {2,0,2,1,1,0};
    int arrsz = ARRLEN(arr);

    bubble_sort(arr, arrsz);
    prt(arr, arrsz);
}