---
title: 存储引擎 执行流程
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

# 1 事务的概念
## 1.1 事务的特性
事务的4个特性：
- 原子性 Automicity
- 一致性 Consistency
- 隔离性 Isolation
- 持久性 Durability

## 1.2 隔离性
不同隔离级别下，可能出现的异常如下：（T 表示可能）

| 异常      | 读未提交 | 读已提交 | 可重复读 | 可串行化 | 解释 |
|-|-|-|-|-|-|
脏读        | T | . | . | . | 读其他事务未提交数据
不可重复读  | T | T | . | . | 2次读取数据不一致
幻读        | T | T | T | . | 2次读取数据条数不一致

postgresql默认的隔离级别是读已提交
```sql
show default_transaction_isolation;
    default_transaction_isolation
    -------------------------------
    read committed
```

## 2 事务的实现原理
为实现事务，有3种并发控制机制：
- MVCC: Multi-Version Concurrency Control, 多版本并发控制
- S2PL: Strict 2-Phase Lock
- OCC: Optimistic Concurrency Control

postgresql使用MVCC机制，主要原理如下：
- 为每个事务分配一个递增的ID
- 数据库保存每个事务的提交状态
- 事务生成tuple时，tuple携带事务ID（在postgresql中，1个tuple存储1行数据）
- 事务删除tuple时，tuple携带事务ID
- 事务查询tuple时，根据tuple时携带的ID，查询对应事务的提交状态，判断tuple是否可见

以下是MVCC的一个简单的示例：
1. 开始事务a，数据库为a分配ID1，并保存提交状态：{1:未提交}
2. 开始事务b，数据库为b分配ID2，并保存提交状态：{1:未提交，2:未提交}
3. 事务a写入数据INSERT INTO t1 VALUES ('a')，数据库存储tuple时，同时存储ID: {1, 'a'}
4. 事务b查询数据SELECT * FROM t1，获取tuple {1, 'a'}，查询ID为1的事务状态，发现该事务未提交，则该tuple不可见

## 2.1 分配事务ID
- 事务ID类型：postgresql使用TransactionId表示事务ID，类型为uint32，简写为xid
- 事务Id分配时机：
    - 场景一：执行INSERT等语句，在写入数据时，才分配事务ID，分配时机如下：
    ```c
    exec_simple_query
        PortalRun
            ProcessQuery
                ExecutorRun
                    standard_ExecutorRun
                        ExecutePlan
                            ExecProcNode
                                ExecModifyTable
                                    ExecInsert
                                        heap_insert
                                            GetCurrentTransactionId
                                                AssignTransactionId
                                                    TopTransactionStateData->transactionId GetNewTransactionId
                                                        /* 获取事务ID */
                                                        ExtendCLOG /* 记录事务状态 */
                                                        ExtendCommitTs
                                                        ExtendSUBTRANS
                                                    XLogInsert(RM_XACT_ID, XLOG_XACT_ASSIGNMENT)
    ```
    - 场景二：执行SELECT语句，无需生成事务ID。只需在执行语句时，获取一次所有事务的状态即可（快照），将来的事务不在事务状态内。根据快照判断tuple可读性即可。
    - 场景三：执行BEGIN语句，并不会立刻分配事务ID，在事务中执行SELECT等也不会分配事务ID。
    ```c
    exec_simple_query
        PortalRun
            PortalRunMulti
                PortalRunUtility
                    ProcessUtility
                        standard_ProcessUtility
                            BeginTransactionBlock
                                /* do nothing */
    ```
- 事务ID持久化：在分配事务ID时，会生成xlog记录分配事务ID的操作

## 2.2 保存事务状态
事务状态分为2部分：快照记录了所有进程当前处理的事务ID，提交日志记录了所有事务的状态
### 2.2.1 事务ID
1. 全局事务ID
```bash
 allProcs            allPgXact # 记录xid
+--------+          +--------+
| PGPROC |   ---    | PGXACT |
+--------+          +--------+
| PGPROC |   ---    | PGXACT |
+--------+          +--------+
| PGPROC |   ---    | PGXACT |
+--------+          +--------+
| ...    |   ---    | ...    |
+--------+          +--------+

struct PGPROC {
    ...
    int pgprocno;
    ...
};

struct PGXACT {
    TransactionId xid;
    TransactionId xmin; /* 开始事务时，最小的xid */
    ...
};
```

### 2.2.2 事务快照
查看当前的事务快照：
```sql
SELECT txid_current_snapshot();
```

一、快照的内容
获取事务快照的函数是GetSnapshotData，该函数会获取所有进程中当前事务的xid
```python
GetSnapshotData
    snapshot->xmax = ShmemVariableCache->latestCompletedXid
    for idx in procArray: # 遍历 allPgXact，获取所有xid
        pgprocno = procArray[idx]
        PGXACT *pgxact = &allPgXact[pgprocno]
        xid = pgxact->xid
        snapshot->xip[count++] = xid
        xin = MIN(xid, xin)
    snapshot->xcnt = count
    snapshot->xin = xin
```

- 二、获取快照的时机
每执行一条SQL时，都会获取一次快照。获取快照的函数为：GetTransactionSnapshot。
如果是
```python
exec_simple_query
    start_xact_command # 1. 简单的初始化
        StartTransactionCommand
            StartTransaction
    if analyze_requires_snapshot # 2. 如果分析 INSERT/SELECT/UPDATE/DELETE，需要快照
        PushActiveSnapshot(GetTransactionSnapshot)
            if IsolationUsesXactSnapshot： # XactIsoLevel >= XACT_REPEATABLE_READ
                return CurrentSnapshot # 3. 如果是可重复读级别及以上，获取旧快照
            GetSnapshotData # 4. 如果是读已提交，则获取新快照
                xmax
                for pgprocno in procArray:
                    xip[count++] = allPgXact[pgprocno]->xid
                    xmin
    pg_analyze_and_rewrite
    pg_plan_queries
    PopActiveSnapshot # 3. 生成计划后，删除分析阶段的快照

    PortalStart
        if PORTAL_ONE_SELECT:
            PushActiveSnapshot(GetTransactionSnapshot) # 4. 如果是SELECT语句，执行计划初始化阶段，生成快照，并将快照保存在执行计划中
            CreateQueryDesc
                QueryDesc->snapshot = RegisterSnapshot(GetActiveSnapshot)
            ExecutorStart
                standard_ExecutorStart
                    RegisterSnapshot(QueryDesc->snapshot)
            PopActiveSnapshot
    PortalRun
        if PORTAL_ONE_SELECT: # 5. SELECT语句无需重新获取快照，Scan使用的快照，是第4步生成的
        if PORTAL_MULTI_QUERY:
            PortalRunMulti
                GetTransactionSnapshot #6. 对于INSERT/DELETE/UPDATE等语法，在执行阶段才获取快照
                    ProcessQuery
```

- 三、使用快照的时机
```python
exec_simple_query
    start_xact_command 
    pg_analyze_and_rewrite
    pg_plan_queries

    PortalStart
        if PORTAL_ONE_SELECT:
            PushActiveSnapshot(GetTransactionSnapshot) # 4. 如果是SELECT语句，执行计划初始化阶段，生成快照，并将快照保存在执行计划中
            RegisterSnapshot(GetActiveSnapshot)
            PopActiveSnapshot
    PortalRun
        if PORTAL_ONE_SELECT:
            PortalRunSelect
                PushActiveSnapshot(QueryDesc->snapshot) # 5. SELECT语句无需重新获取快照，Scan使用的快照，是第4步生成的
                ExecutorRun
                    standard_ExecutorRun
                        ExecutePlan
                            ExecProcNode
                                ExecSeqScan
                                    SeqNext
                                        heap_getnext
                                            heapgettup
                                                HeapTupleSatisfiesVisibility
                                                    HeapTupleSatisfiesMVCC
                                                        if !HeapTupleHeaderXminCommitted:
                                                            XidInMVCCSnapshot(HeapTupleHeaderGetRawXmin(tuple), HeapScanDesc->rs_snapshot)
                                                                # tuple 不可见
                PopActiveSnapshot()
        if PORTAL_MULTI_QUERY:
            PortalRunMulti
                GetTransactionSnapshot #6. 对于INSERT/DELETE/UPDATE等语法，在执行阶段才获取快照
                RegisterSnapshot
                PushCopiedSnapshot

                ProcessQuery
                    ExecutorStart
                    ExecutorRun
                        ExecutePlan
                            ExecProcNode
                                ExecModifyTable
                                    ExecInsert
                                        heap_insert # INSERT 好像用不到快照
                                    ExecUpdate
                                        heap_update
                                            HeapTupleSatisfiesUpdate
                                                TransactionIdIsInProgress(HeapTupleHeaderGetRawXmin(tuple))
                                            HeapTupleSatisfiesVisibility
                                            HeapTupleHeaderAdjustCmax
                                    ExecDelete
                                        heap_delete
                                            HeapTupleSatisfiesUpdate
                                            HeapTupleSatisfiesVisibility
                                            HeapTupleHeaderAdjustCmax
    PortalDrop
    finish_xact_command
```

### 2.2.3 提交日志
- 一、提交日志
事务的提交日志中，记录了所有事务的状态：是否在运行、是否已提交等
由于事务ID是从1开始递增，可用位图表示所有事务的状态，位图的索引即事务ID
提交日志也需持久化，位图被划分存储在Page中，Page存储在提交日志文件中
内存中也会缓存一些Page，使用LRU缓存Page
```c
ClogCtlData / ClogCtl
+------+------+------+------+
| Page | Page | Page | Page |
+------+------+------+------+

XidStatus
#define TRANSACTION_STATUS_IN_PROGRESS		0x00
#define TRANSACTION_STATUS_COMMITTED		0x01
#define TRANSACTION_STATUS_ABORTED			0x02
#define TRANSACTION_STATUS_SUB_COMMITTED	0x03
```

- 二、更新提交日志
在分析或执行阶段，首次获取事务ID时，会扩展提交日志。在执行结束后，会根据事务状态更新提交日志。
```python
exec_simple_query
    start_xact_command

    # 执行解析、分析、执行等步骤
    PortalRun
        ...
            heap_insert
                GetNewTransactionId
                    /* 获取事务ID */
                    ExtendCLOG /* 记录事务状态 */

    finish_xact_command
        CommitTransactionCommand # autovacuum / walsender等都会调用
            s = CurrentTransactionState
            if TBLOCK_STARTED:
                CommitTransaction
                    ProcArrayEndTransaction # 清理进程当前处理的xid
                        pgxact = &allPgXact[pgprocno]
                        ProcArrayEndTransactionInternal
                            pgxact->xid = InvalidTransactionId;
                            pgxact->xmin = InvalidTransactionId
                    RecordTransactionCommit / xact_redo_commit # 更新提交日志
                        XactLogCommitRecord
                            XLogInsert(RM_XACT_ID, info) # 生成xlog，记录事务已提交
                        TransactionIdCommitTree
                            TransactionIdSetTreeStatus(TRANSACTION_STATUS_COMMITTED)
            if TBLOCK_BEGIN:
                不提交事务
                s->blockState = TBLOCK_INPROGRESS
            if TBLOCK_INPROGRESS:
                CommandCounterIncrement

# 其他更新事务状态的场景，不额外介绍
TransactionIdAsyncCommitTree
    TransactionIdSetTreeStatus(TRANSACTION_STATUS_COMMITTED)
TransactionIdAbortTree
    TransactionIdSetTreeStatus(TRANSACTION_STATUS_ABORTED)
```

- 三、使用提交日志
在更新或删除一条tuple前，会先检查是否有事务删除tuple，如果有则检查事务是已提交。判断是否已提交便是根据提交日志判断。下文会继续介绍判断逻辑
```python
heap_update
    HeapTupleSatisfiesUpdate
        TransactionIdDidCommit
            TransactionLogFetch
    UpdateXmaxHintBits
        XidStatus = TransactionIdDidCommit
        return xidstatus == TRANSACTION_STATUS_COMMITTED
heap_delete
    UpdateXmaxHintBits
        TransactionIdDidCommit
```

## 2.3 Tuple携带事务ID
以下是tuple的结构：
```c
typedef struct { /* ignore Union */
    TransactionId t_xmin; /* 生成tuple的事务ID */
    TransactionId t_xmax; /* 删除tuple的事务ID，默认为0 */
    CommandId	t_cid;
    ItemPointerData t_ctid; /* 更新tuple时，新数据的事务ID，默认为0 */

    uint16 t_infomask2;
    uint16 t_infomask;
    uint8 t_hoff;
    bits8 t_bits[FLEXIBLE_ARRAY_MEMBER];

    /* tuple data */
} HeapTupleHeader;
```

## 2.4 判断Tuple可见性
内容太多，学得头大，下次继续