#include <stdio.h>
#include <string.h>

#define SWAP(arr, i, j)      \
    do {                     \
        int tmp_ = arr[(i)]; \
        arr[(i)] = arr[(j)]; \
        arr[(j)] = tmp_;     \
    } while (0)
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

// ----------------------------------------------------------------------------

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
    int le = left;
    int ri = right;
    int pivot = arr[le];

    /* 找到pivot，比pivot大的放在右边，比pivot小的放左边 */
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

/* 3 归并排序 */
void sort_merge(int *arr, int left, int right)
{
    if (left >= right) {
        return;
    }

    /* 先排序左边，再排右边，最会归并 */
    int mid = (left + right) / 2;
    sort_merge(arr, left, mid);
    sort_merge(arr, mid + 1, right);

    int merge[right - left + 1];
    int mergelen = 0;
    int i = left;
    int j = mid + 1;
    while (i <= mid && j <= right) {
        merge[mergelen++] = arr[i] < arr[j] ? arr[i++] : arr[j++];
    }
    while (i <= mid) {
        merge[mergelen++] = arr[i++];
    }
    while (j <= right) {
        merge[mergelen++] = arr[j++];
    }
    memcpy(arr + left, merge, sizeof(int) * mergelen);
}

/* 4 堆排序 */
void sort_heap(int *arr, int len)
{
#define ROOT_NODE(child) (int)(((child) - 1) / 2)

    int *heap = arr;
    int heapsz = 0;
    int father;
    int child;

    for (heapsz = 1; heapsz < len; heapsz++) {  // 自底向上，构造大顶堆
        for (child = heapsz - 1, father = ROOT_NODE(child); child != 0; child = father, father = ROOT_NODE(child)) {
            if (heap[father] < heap[child]) {
                SWAP(heap, child, father);
            }
        }
    }

#define LEFT_NODE(father) (2 * (father) + 1)
#define RIGHT_NODE(father) (2 * (father) + 2)

    int left;
    int right;
    int max;
    for (; heapsz > 0; swap(heap, 0, heapsz - 1), heapsz--) { // 交换首尾，自顶向下，重构大顶堆
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
}

/* e.g: (0xabcd, 0xf00) -> (0xb) */
int num_digit(int num, int digit)
{
    int shift;
    for (shift = 4; (digit >> shift) != 0; shift += 4);
    shift -= 4;
    return (num & digit) >> shift;
}

/* 5 基数排序 */
void sort_base(int *arr, int len)
{
    // 先排个位数，再排十位数，再排百位数
    int max = arr[0];
    int i;
    for (i = 0; i < len; max = max > arr[i] ? max : arr[i], i++) {}
    int maxdig;
    for (maxdig = 0xf; (max & maxdig) != 0; maxdig = (maxdig << 4)) {}

    int dig;
    int hash[0xf][len];
    int hashlen[0xf] = {0};
    for (dig = 0xf; dig < maxdig; dig = (dig << 4)) { // 分别获取个位数、十位数、百位数、...
        for (i = 0; i < len; i++) { // 分桶
            int num = num_digit(arr[i], dig);
            hash[num][hashlen[num]++] = arr[i];
        }

        int alen = 0;
        int j;
        for (i = 0; i < 0xf; i++) { // 桶归并
            for (j = 0; j < hashlen[i]; j++) {
                arr[alen++] = hash[i][j];
            }
        }
    }
}

int main()
{
    int arr[100] = {5, 9, 7, 10, 3, 1};
    sort_base(arr, 6);
    prt(arr, 6);
    return 0;
}