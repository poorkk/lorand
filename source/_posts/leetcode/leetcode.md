---
title: 算法 leetcode
date: 2022-09-25 16:03:37
categories:
    - 算法
tags:
    - 算法
---

[toc]

# 1 概述
## 1.1 分类
- 数组
    - 排序
    - 滑动窗口
    - 去重
    - 碰撞指针：回文串
- 链表
    - 反转链表
- 二叉树
    - 递归
- 栈
- 队列

- 动态规划
- 背包问题

## 1.2 解法
**数组**
1. 移动0：数组中，将0移到数组末尾
    - 冒泡排序：把0冒泡到末尾（不改变元素顺序）
    ```c
    for (i = 0; i < arrsz; i++) {
        if (arr[i] == 0) {
            ...
        }
    }
    ```
2. 移除元素：数组中，将指定val移除，返回移除后的数组
    - 冒泡排序：把val冒泡到末尾
    - 首尾双指针：把val与末尾交换（可改变元素顺序）
    ```c
    for (i = 0; i < arrsz; i++) {
        if (arr[i] == val) {
            ...
        }
    }
    ```
3. 删除重复元素：有序数组中，删除重复项
    - 双指针：start和end分别指向重复元素首末，然后start = end + 1
    ```c
    while (start < arrsz && end < arrsz) {
        while (end < arrsz && arr[end] == arr[start]) {
            end++;
        }
        arr[newsz++] = arr[start];
        start = end;
    }
    ```
4. 颜色分类：数组中，仅包含0,1,2，排序
    - 冒泡排序：把大数冒泡到后面
    ```c
    for (i = 0; i < arrsz - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            SWAP(arr, i, i + 1);
        }
    }
    bubble_sort(arr, arrsz - 1);
    ```
    - 双指针p0,p1,pcur：覆盖 + 补偿 （仅扫描一次）：首先，p0与p1扩展空间，然后交换
    ```c
    0 1 2 0 0
    0
      1
         cur : p0++, p1++, swap(p0, p1), swap(pcur, p0)
      0 (先扩展p0)
        1 (再扩展p1，最后交换)
    ```
5. 第K个最大数：数组中，第K个最大数
    - 快速排序
    - 堆排序：建堆，删除k-1次堆顶
6. 两数之和：有序数组中，查找和为target的2个数
    - 二分查找
    - 首尾双指针
    - 哈希表
7. 回文字符串：字符串中，排除空格特殊字符和大小写，正读和反读一样
    - 首尾双指针
8. 

# 2 数组
## 2.1 所有0移动到数组的末尾
> https://leetcode.cn/problems/move-zeroes/
```c
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
```

## 2.2 移除数组指定元素
```c
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
```

## 2.3 移除数组重复元素
```c
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
```

## 2.4 移除数组数量大于2的元素
```c
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
```

## 2.5 数组重复元素排序
```c
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

void bubble_sort1(int *arr, int arrsz) // 双指针，覆盖 + 补偿
{
    if (arrsz <= 1) {
        return;
    }
    
    int p0 = 0;
    int p1 = 0;
    int i;
    /*
     * 0 0 1 1 2 2 1 0 0 1
     *   -
     *       +
     *             ^   
     */
    for (i = 0; i < arrsz - 1; i++) {
        if (arr[i] == 0) {

        }
        if (arr[i] > arr[i + 1]) {
            SWAP(arr, i, i + 1);
        }
    }
    bubble_sort(arr, arrsz - 1);
}
```

## 2.6 第K个最大数
```c
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
```

## 2.7 最大的K个数
- 维护小顶堆
- if num > heap[0] : heap[0] = num, shift(heap)

## 2.8 两数之和


# 参考
- https://leetcode.cn/circle/article/QxxqIF/