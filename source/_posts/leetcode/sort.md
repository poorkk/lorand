---
title: 算法 排序
date: 2022-09-25 16:03:37
categories:
    - 算法
tags:
    - 算法
---

# 1 概述
## 1.1 衡量标准

## 1.2 分类
| 算法 | 思想 | 最好时间复杂度 | 最坏时间复杂度 | 时间复杂度 | 空间复杂度 | 稳定性
|-|-|-|-|-|-|-|
冒泡排序 | 比较、交换 | O(n) | O(n^2) | O(n^2) | O(1) | 稳定，值相等不交换
插入排序 | 比较、移动 | O(n) | O(n^2) | O(n^2) | 

```c
/* 冒泡排序 */
void sort_bubble(int *arr, int arrlen)
{
    int end;
    int tmp;
    int i;

    end = arrlen - 1;
    for (end = arrlen - 1; end > 1; end--) {
        for (i = 0; i < end - 1; i++) {
            if (arr[i] < arr[i + 1]) {
                tmp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = tmp;
            }
        }
    }
}
```

```c

```