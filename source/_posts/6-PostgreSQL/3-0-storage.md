---
title: PostgreSQL 3-1 Heapam
date: 2022-09-25 16:31:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

# 1 概述
本博客中，将以循序渐进的方式，依次介绍PostgreSQL存储模块有哪些子模块，以及每个子模块的基础功能。针对每个提到的子模块，之后会单独分别写博客详细介绍。
不过，俺很年轻，学的不多，能力有限，有错的、缺的、乱的内容，都是合理的。

## 3.1 FileNode
- PostgreSQL如何存储数据？
通常，当用户创建1个表时，PostgreSQL会生成1个与表对应的数据文件。当用户向表中写入数据时，PostgreSQL会将数据写到表对应的数据文件中。

## 3.2 Page
> 本博客仅介绍核心原理，不考虑以下情形：
>   - 大数据量：如果向1个表写入狠多数据，该表对应的数据文件大小达到1G时，该表会生成第2个数据文件来存数据。依次类推，表中数据量较大时，1个表可能有很多个数据文件。1个表最多可拥有32T的数据文件。本博客不关心这些，只考虑1个表对应1个文件的情形。
>   - 分区表：如果创建分区表，每个分区的数据文件都是独立的。本博客不关心分区表。
>   - 段页式表：如果1个表对应1个或多个文件，当表的数量过多时，文件的数量也很多，Oracle支持将多个表的数据存储在同1个文件中，以缩减文件数量。本博客不关心Oracle。

- PostgreSQL如何组织数据？
1个数据文件，由多个大小为8k的数据页Page组成。在示例的语法`INSERT INTO t1 VALUES (1, 'data1')`中，`(1, 'data1')`是一行数据，在PostgreSQL中，1个Tuple存储1行数据。1个Page可以存储多个Tuple。每已有Page无法容纳新的Tuple时，PostgreSQL便会生成新的Page。
如下图所示，向1个表INSERT一些数据后，该表的物理文件大概如下图所示：
[图]

## 3.2 Smgr
- PostgreSQL如何读写数据？
上文提到，PostgreSQL采用Page存储数据。1个Page可存储多个Tuple。
PostgreSQL从数据文件读数据时，1次至少读1个Page，假设1个Page有100个Tuple，但只需要第3个Tuple，还是会1次读1整个Page，写数据文件同理。总而言之，读写数据文件时，以Page为粒度，1次读或写至少1个Page，读到Page之后，再在Page里写或读Tuple。

- PostgreSQL读取数据的接口？
Smgr模块提供操作文件的接口，它简单封装了`open(), write(), read()`等接口，让数据库以Page为粒度进行读写。
Smgr模块读写数据的一些接口定义如下：

```c

```

> vfd模块：
>   - 数据库管理很多文件，而1个进程最多只能打开64个文件，为解决数据库无法打开过多文件的限制，Smgr模块底层还有1个VFD模块，它的机制比较常见，本博客不关心VFD。

## 3.3 Buffer
- PostgreSQL如何提高读写效率？
Buffer模块缓存了一些Page，降低PostgreSQL从磁盘读取Page的次数。所有表的Page，都可缓存在Buffer中。

## 3.5 Wal
- PostgreqSQL如何解决高可用问题？

## 3.6 Transcation
- PostgreSQL如何解决高并发问题？

## 3.3 Fsm
- PostgreSQL如何选择Tuple的存储位置？

## 3.9 Vm

## 3.9 Heapam

## 3.10 Index

## 3.11 Vacuum

## 3.12 Backup

## 3.13 Standby