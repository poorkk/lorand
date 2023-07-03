#include <stdio.h>
#include <string.h>

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)
#define ARRLEN(arr) (int)(sizeof((arr))/sizeof((arr)[0]))
void swap(int *arr, int i, int j)
{
    int tmp;
    tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}
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

int max_k(int *arr, int arrsz, int k) // 第k个最大数：构造大顶堆，删除k-1次栈顶
{
#define ROOT_NODE(child) (int)(((child) - 1) / 2)

    int *heap = arr;
    int heapsz;
    int father;
    int child;

    if (k > arrsz) {
        return -1;
    }

    for (heapsz = 1; heapsz <= arrsz; heapsz++) {  // 自底向上，构造大顶堆
        for (child = heapsz - 1, father = ROOT_NODE(child); child != 0; child = father, father = ROOT_NODE(child)) {
            if (heap[father] < heap[child]) {
                SWAP(heap, child, father);
            }
        }
    }
    heapsz--;

#define LEFT_NODE(father) (2 * (father) + 1)
#define RIGHT_NODE(father) (2 * (father) + 2)

    int left;
    int right;
    int max;
    int tarheapsz = arrsz - k + 1;
    while (heapsz > tarheapsz) { // 交换首尾，自顶向下，重构大顶堆
        prt(heap, heapsz);
        SWAP(heap, 0, heapsz - 1);
        heapsz--;

        for (father = 0, left = LEFT_NODE(father), right = RIGHT_NODE(father); father < heapsz; left = LEFT_NODE(father), right = RIGHT_NODE(father)) {
            if (left >= heapsz) {
                break;
            }
            max = left < heapsz && right < heapsz ? (heap[right] > heap[left] ? right : left) : left;
            if (heap[father] < heap[max]) {
                SWAP(heap, father, max);
                father = max;
                continue;
            }
            break;
        }
    }

    return heap[0];
}

int find_sum(int *arr, int arrsz, int tar, int *ressz)
{
    int *res;

    res = (int *)malloc(2 * sizeof(int));

    return res;
}

int main()
{
    // int *arr = nums;
    // int arrsz = numsSize;
    int arr[] = {3,2,3,1,2,4,5,5,6};
    int arrsz = ARRLEN(arr);
prt(arr, arrsz);
    int a = max_k(arr, arrsz, 4);
    prt(arr, arrsz);
    printf("%d\n", a);
}