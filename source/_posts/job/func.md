---
title: 工作 透明加密
date: 2023-01-01 16:03:37
categories:
    - 工作
tags:
    - 工作
---

# 功能
# 一 语义分析
## 1 语义分析 - 客户端
1. 约束：明文与密文运算
> 明文type != 密文type

2. 约束：密文与密文运算，但密钥不同
> 客户端 检查密钥ID不同

3. 客户端语义分析 密钥不同
约束
- A.c UNION B.c
- A.c JOIN B.c
原因
- 新表进行计算复杂: JOIN
- 排序、
场景
- INSERT .          SELECT-UNION
- CREATE TABLE AS . SELECT-UNION
- SELECT-UNION .    INTO
- SELECT-UNION .    JOIN .

难点与挑战：
    1. 识别密文运算场景
    2. 学习数据库内核多个模块：语法解析、语义分析、查询重写、查询优化、计划生成、计划执行
    3. 学习机密计算原理：内存管理、机密计算交互
    4. 设计密文运算方案：功能、性能、可扩展性