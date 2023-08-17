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
## 1.1 概述
**索引方式**
- 唯一索引：不能出现重复的值
- 主键索引：主键自动创建唯一索引
- 多属性索引：多个列（最多32列）
- 部分索引：

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
    
    # 2 扫描基表，将tuple存入spool中
    reltuples = IndexBuildHeapScan(heap, index, indexInfo, btbuildCallback, &buildstate)
        IndexBuildHeapRangeScan() {
            # 为扫描基表，创建一个执行器
            EState estate = CreateExecutorState()

            HeapScanDesc scan = heap_beginscan_strat(heap)
            while heapTuple = heap_getnext(scan):
                ExecStoreTuple(heapTuple)
                btbuildCallback(indexRelation, heapTuple, buildstate) {
                    if tupleIsAlive:
                        _bt_spool(buildstate.spool, heapTuple)
                    else:
                        _bt_spool(buildstate.spool2, heapTuple)
                }
        }
    
    # 3 排序spool中的tuple，并构建b+树
    _bt_leafbuild(buildstate.spool, buildstate.spool2) {
        # 3.1 排序
        tuplesort_performsort(spool.sortstate)
        tuplesort_performsort(spool2.sortstate)

        # 3.2 遍历spool，并构建b+树
        _bt_load(wstate, spool, spool2) {
            IndexTuple itup = tuplesort_getindextuple(spool)
            IndexTuple itup2 = tuplesort_getindextuple(spool2) # 如果spool2不为空，与spool归并

            keysz = RelationGetNumberOfAttributes(index)
            for (i = 0; i < keysz; i+=1)
                PrepareSortSupportFromIndexRel(index,)
            
            for (;;)
                for(i = 1; i < keysz; i+=1)
                    ApplySortComparator(itup, itup2)
                # 3.2.1 生成第1个index page
                if state == NULL:
                    tate = _bt_pagestate(wstate, 0)
                # 3.2.2 向index page插入tuple
                _bt_buildadd(wstate, state, itup)
                itup = tuplesort_getindextuple(spool)
            
            # 3.2.3 更新metapage信息
            _bt_uppershutdown(wstate, state)
        }
    }
    
    # 4 清理资源
    _bt_spooldestroy(buildstate.spool)
    if uildstate.spool2:
        bt_spooldestroy(buildstate.spool2)
```

## 2.2 aminsert
```python
btinsert()
    itup = index_form_tuple(rel)
    _bt_doinsert(rel, itup, heapRel)
        itup_scankey = _bt_mkscankey()

        # 在buffer中搜索位置
        stack = _bt_search(rel, itup_scankey, &buf)
            page = BufferGetPage(buf)
            opaque = (BTPageOpaque) PageGetSpecialPointer(page)
            low = P_FIRSTDATAKEY(opaque)
            high = PageGetMaxOffsetNumber(page)
            while (high > low) # 二分查找
                OffsetNumber mid = low + ((high - low) / 2)
                _bt_compare(rel, page, mid)
                low = mid + 1
                high = mid
            if P_ISLEAF(opaque)
                return low
            return OffsetNumberPrev(low)

        buf = _bt_moveright(rel, buf, itup_scankey, stack)
        _bt_findinsertloc(rel, buf, &offset, itup_scankey, itup, stack, heapRel)
        _bt_insertonpg(rel, buf, stack, itup, offset)

        _bt_freestack(stack)
        _bt_freeskey(itup_scankey)
```