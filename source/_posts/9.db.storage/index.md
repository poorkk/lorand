---
title: 存储引擎 index
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

[toc]

# 1 索引概述
## 1.1  执行器执行流程
### 1 索引创建
```python
DefineIndex()
    index_create() # 向系统表中写入tuple
    index_open()
    index_build()
        ambuild -> OidFunctionCall3()
    index_close()
    validate_index()
```

### 2 索引插入
```python
ExecInsert()
    heap_insert()
    ExecInsertIndexTuples()
        index_insert()
            aminsert -> FunctionCall6()
```

### 3 索引更新
```python
ExecUpdate()
    heap_update()
    ExecInsertIndexTuples()
            index_insert()
```

### 4 索引删除
```python
ExecDelete()
    heap_delete()
```

### 5 索引扫描
```python
ExecIndexScan()
    IndexNext()
        index_getnext()
            index_getnext_tid()
                amgettuple -> FunctionCall2()
            index_fetch_heap()
```

## 1.2 索引模块接口
pg_am系统表存储统一接口
| 类型 | btree | hash | gist | gin | spgist | brin |
|-|-|-|-|-|-|-|
aminsert    | btinsert      |
ambeginscan | btbeginscan   |
amgettuple  | btgettuple    |
amgetbitmap | btgetbitmap   |
amrescan    | btrescan      |
amendscan   | btendscan     |
ammarkpos   | btmarkpos     |
ambuild     | btbuild       |
ambuildempty| btbuildempty  |
ambulkdelete| btbulkdelete  |
amvacuumcleanup| btvacuumcleanup|
amcanreturn | btcanreturn   |
amcostestimate | btcostestimate |
amoptions   | btoptions     |

# 2 索引实现
## 2.1 ambuild
```python
btbuild(Relation heap, Relation index, IndexInfo indexinfo, IndexBuildResult result)
    BTBuildState buildstate

    # 1 初始化pool
    buildstate.spool = _bt_spoolinit(heap, index)
    if indexInfo.ii_Unique:
        buildstate.spool2 = _bt_spoolinit(heap, index)
    
    # 2 扫描
    reltuples = IndexBuildHeapScan(heap, index, indexInfo, btbuildCallback, &buildstate)
        IndexBuildHeapRangeScan() {
            # 为扫描基表，创建一个执行器
            EState estate = CreateExecutorState()

            HeapScanDesc scan = heap_beginscan_strat(heap)
            while heapTuple = heap_getnext(scan):
                
        }
            



```