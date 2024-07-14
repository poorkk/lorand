---
title: PostgreSQL 3-0 存储模块
date: 2022-09-25 16:31:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

> **摘要**：本博客以循序渐进的方式，依次简要介绍PostgreSQL存储模块的各个子模块的基础功能，提及内容适合数据库内核初学者。本博客旨在介绍核心原理，以便于理解为首，部分内容可能不会100%准确与具体，针对每个提及的子模块，之后会分别单独写博客详细介绍
> **提示**：作者业余时间不多，年纪不大，能力不高，有错的、缺的、乱的内容，都是合理的

[toc]

# 1 概述
# 1.1 PostgreSQL存储模块工作原理
本博客仅讨论存储模块的关键原理。简而言之，PostgreSQL将数据合理地组织并存储在磁盘上，并向用户提供SQL接口，让其读写磁盘上的指定数据。以下是PostgreSQL简单工作流程：
1. **安装PostgreSQL**：安装前，用户需指定一个目录作为PostgreSQL的数据目录。安装时，PostgreSQL会在数据目录下生成很多子目录以及文件
2. **启动PostgreSQL**：等待用户向PostgreSQL建立连接并发送请求
3. **连接PostgreSQL**：用户连接PostgreSQL后，可向其发送SQL语句，让其执行。以常见的SQL语句为例，PostgreSQL的执行原理如下：
    1. 创建表：PostgreSQL接收以下SQL后，在数据目录下生成1个用于存储`t1`数据的数据文件
    ```sql
    CREATE TABLE t1 (c1 INT, c2 TEXT);
    ```
    2. 写数据：PostgreSQL接收以下SQL后，将SQL语句中的数据`(1, 'data1')`写入上一步创建的`t1`数据文件
    ```sql
    INSERT INTO t1 VALUES (1, 'data1');
    ```
    3. 读数据：PostgreSQL接收以下SQL后，从`t1`的数据文件中，读取数据，并发送给用户
    ```sql
    SELECT * FROM t1;
    ```

# 2 执行模块
占个位先，以后介绍执行模块

# 3 存储模块
## 3.1 Oid
- PostgreSQL如何识别与查找表？
    - 在PostgreSQL中，每个对象都有唯一的id，叫Oid，包括表，即每个表也有1个唯一的Oid
    - 创建1个表时，PostgreSQL为该表生成对应的数据文件，默认情况下，文件的名字即为表的Oid，Oid是一个整数，比如17001。
    > PostgreSQL支持一些将表数据挪至新文件的功能，此时表的名字可能会变成其他整数，暂不考虑该场景。

## 3.2 Page
- PostgreSQL如何组织数据？
    - 1个数据文件，由1个或多个大小为8192字节（即8k）的数据页（Page）组成。
    - 在SQL语法`INSERT INTO t1 VALUES (1, 'data1')`中，`(1, 'data1')`是表中的1行数据。
    - 在PostgreSQL中，1个Tuple存储1行数据，数据越长，Tuple越长。1个Page可以存储多个Tuple。

如下图所示，向1个表INSERT一些数据后，表数据文件会有多个Page，所有Page大小一致，均为8192字节。Tuple按顺序从Page尾部向Page头部排列。当前Page无法存放新Tuple时，生成新的Page。以此类推，数据越多，Page越多。

{% asset_img page.png %}

> **提示1**：本篇博客旨在介绍基础原理，暂不关注细枝末节的问题：比如1条数据的长度可能超过8k，1个表可能有多个数据文件：
>   - 大数据量：如果向1个表写入狠多数据，该表对应的数据文件大小达到1G时，该表会生成第2个数据文件来存数据。依次类推，表中数据量较大时，1个表可能有很多个数据文件。1个表最多可拥有32T的数据文件。本博客不关心这些，只考虑1个表对应1个文件的情形。
>   - 分区表：如果创建分区表，每个分区的数据文件都是独立的。本博客不关心分区表。
>   - 段页式表：如果1个表对应1个或多个文件，当表的数量过多时，文件的数量也很多，Oracle支持将多个表的数据存储在同1个文件中，以缩减文件数量。本博客不关心Oracle。

## 3.3 Smgr
- PostgreSQL如何读写数据？
上文提到，PostgreSQL采用Page存储数据，1个Page可存储多个Tuple。
PostgreSQL从磁盘读取数据文件中的数据时，1次至少读1个Page，假设1个Page有100个Tuple，但只需要第3个Tuple，还是会1次读1整个Page，写数据文件同理。该机制与操作系统的IO机制一样。总而言之，读写数据文件时，以Page为粒度，1次读或写至少1个Page，读到Page之后，再在Page里写或读Tuple。

{% asset_img smgr.png %}

Smgr模块提供操作文件的接口，它简单封装了`open(), write(), read()`等接口，让数据库以Page为粒度进行读写。
Smgr模块读写数据的3个关键接口定义如下：
```c
/* 1 创建或打开1个文件, rnode和文件路径和文件名有关，以后介绍 */
.. smgropen(RelFileNode rnode);
/* 2 向文件中写入1个Page。其中，blocknum即Page编号，从1开始的整数；buffer即长度为8k的Page数据 */
smgrwrite(.., BlockNumber blocknum, char *buffer, ..);
/* 3 从文件中读取1个Page。其中，blocknum即Page编号；buffer即用于存放Page的长度为8k的缓存区 */
smgrread(.., BlockNumb blocknum, char *buffer);
/* 4 其他之后介绍 */
```
不同操作系统，底层读写文件的接口可能不一致，但是，存储模块仅需调用smgr的接口即可，smgr底层解决了跨平台的问题。

> **VFD模块**：
>   - 1个数据库管理成千上万个文件，1个Linux进程最多只能打开64个文件，为解决数据库无法打开过多文件的限制，Smgr模块底层实现了1个VFD模块，它的机制比较常见，本博客不关心VFD

## 3.4 Buffer
- PostgreSQL如何提高读写效率？
    - 为提高读写效率，PostgreSQL会在内存中缓存一些最近使用的Page，以减少读写磁盘的次数，实现该功能模块即Buffer模块
    - 所有表的Page，都可缓存在Buffer中。读写表的数据时，需指定关键参数：[表数据文件路径、文件内第几个Page]，这些关键参数即可作为查找缓存的建值
    - Buffere中的Page，都在内存中，多个进程或线程可在锁保护的情况下共同操作Page

{% asset_img buffer.png %}

## 3.5 Wal
- PostgreqSQL如何同时解决高可靠与高性能问题？
数据库是一款高可靠软件，需要考虑运行数据库的机器发生断电等特殊情况。
假如，1个用户，在1个事务中，执行以下3条SQL语句，PostgreSQL需确保数据都已被写入磁盘后，才会告诉用户，事务执行成功。
```sql
BEGIN;
INSERT INTO t1 VALUES (1, ‘data1’);
INSERT INTO t2 VALUES (2, ‘data2’);
INSERT INTO t3 VALUES (3, ‘data3’);
COMMIT;
```

上述示例中，执行该事务，要向3个表的3个数据文件，写入3次数据，性能较低。几乎所有关系型数据库，都提供预写日志（Write Ahead Log, WAL）的功能，可有效解决高性能和高可靠的问题。
WAL的基本原理是：
1. 用户执行1条事务，可能需要向多个文件的多个Page中，写多条Tuple。开始时，把Page都放在Buffer中，并操作Page
2. 然后，把这些操作（写了哪个文件，哪个Page，Page内的写入位置，数据本身），合并在一起，生成WAL日志
3. 接下来，将WAL日志写入磁盘后，便可告诉用户，事务已执行成功
4. 最后，再慢慢将Buffer中的Page写入到多个文件中。此时，即使第4步执行到一半，发生机器断电，PostgreSQL重启后，根据第3步已写入磁盘的WAL日志，可找回待写入的数据，并继续完成写如操作

{% asset_img wal.png %}

WAL日志通过重复与提前记录写入操作以及数据等信息，将WAL日志落盘看做事务持久化的，可大幅降低落盘次数，提高性能，解决数据库高可靠和高性能问题。其实，一个事务，可能需要操作的文件类型，以及文件数据数量，都源多于此处的例子，WAL的重要性极高。
WAL日志记录的内容如此丰富，存储模块还有很多其他功能，是基于WAL日志实现的，以后会详细介绍
> 在PostgreSQL中，WAL日志也叫xlog，xlog也叫WAL日志

## 3.6 Transcation
- PostgreSQL如何解决高并发问题？
实际应用场景中，1个数据库，可能需要同时处理成百上千个事务，这些事务，可能会访问同一个表，产生很多访问冲突的问题。以下是一个简单的示例：
```sql
-- 用户1，执行事务1，事务尚未提交
BEGIN;
INSERT INTO t1 VALUES (1, 'data1');

-- 用户2，执行事务2
BEGIN;
SELECT * FROM t1 WHERE c1 = 1;
```

此时，事务2会和事务1发生访问冲突，数据库有不同的事务隔离级别（此处不详细介绍），按常见的隔离级别，事务2应该有以下2个特点：
1. 即使事务1没有提交事务，事务2也可以访问t1表，无需等待
2. 事务1的数据`(1, 'data')`还未提交，事务2应无法访问`(1, 'data')`，即该条数据对事务2不可见

为解决上述问题，包括PostgreSQL在内，大部分关系数据库都有MVCC（此处不详细介绍）事务隔离机制，PostgreSQL实现事务隔离机制如下图所示：（图中的xid即事务id）

{% asset_img transcation.png %}

1. 执行事务的过程中，为每个事务分配1个事务id，事务id是一个整数。先开始的事务，事务id小，后开始的事务，事务id大
2. 同时，PostgreSQL实时记录所有事务的状态：已提交、未提交、...
3. 事务生成Tuple，并将数据封装到Tuple时，会同时将自己的事务id封装至Tuple中
4. 其他事务读到Tuple时，可通过Tuple的事务id及其对应的事务状态等信息，判断Tuple中的数据是否对本事务可见

## 3.7 Fsm
- PostgreSQL如何选择Tuple的存储位置？
上文提到，1个表有多个Page，1个Page可存储多个Tuple，1个Tuple存储1行数据，数据来自`INSERT .. ('data')`等语法，语法中的数据长短不一，有的数据长度可能为0.1k，有的数据长度可能为7k。如果执行`DELETE`等操作，某些Page中的某些Tuple可能跟被删除。
所以，1个表的不同Page中，空闲空间的大小不一致，有的空闲空间大，有的空闲空间小，当处理`INSERT .. ('data')`等语句时，需要选择1个合适的Page，来存储数据。
1个表文件，对应1个FSM文件，FSM文件中，记录了表文件所有Page的空闲空间。当向表中写数据时，可先读取FSM文件，从中查找到1个空间空间合适的Page，在将数据写入到该Page中。

{% asset_img fsm.png %}

## 3.9 Vm
（未完待续）

## 3.10 Heapam

## 3.11 Index

## 3.12 Vacuum

## 3.13 Backup

## 3.14 Standby