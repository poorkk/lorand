---
title: PostgreSQL 3-1 Heapam
date: 2022-09-25 16:31:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

# 1 概述
前置知识：Relation的基本概念，Tuple的基本概念

## 1.1 heapam接口基本介绍
heapam是存储模块对上层提供的数据读写接口，这些接口通常是操作1个relation中的1个或多个tuple。
本文介绍以下4个最常用的接口，即如何向1个relation中写入、删除、更新、读取1个Tuple，接口定义如下：
```c
/* 向relatiion中写入1条tuple */
heap_insert(Relation relation, HeapTuple tup, ...)
/* 从relation中删除1条tuple */
heap_delete(Relation relation, ItemPointer tid, ...)
/* 在relation中，将旧tuple标记删除，并写入1个新tuple */
heap_update(Relation relation, ItemPointer otid, HeapTuple newtup, ...)
/* 从relation中读取数据，每次读取1条数据 */
HeapScanDesc heap_beginscan(Relation relation, ...)
HeapTuple heap_getnext(HeapScanDesc scan)
```

## 1.2 heapam接口基础功能
实际应用场景中，数据库的存储模块需满足以下关键需求：
- 操作多：1个表需要支持数据写入、查询、更新、删除等操作
- 数据多：1个表可能存储亿级以上的数据
- 并发高：短时间内，可能上百个连接同时读写1个表
- 可靠性高：突然发生断电等特殊情况，数据不会发生错乱
- 性能高：使用一些机制，优化数据访问的速度，比如：缓存、锁等

明确存储接口该解决哪些问题后，了解heapam接口时，可大概清楚每一步的目的。

## 1.3 heapam的基本

```c
otherBuffer：update场景，包含旧数据
targetBuffer：插入的页面，默认从上次插入的页面接着插入

Oid heap_insert(Relation relation, HeapTuple tup, CommandId cid, int options, BulkInsertState bistate)
    HeapTuple heaptup = heap_prepare_insert(relation, tup, xid, cid, options)
    Buffer buffer = RelationGetBufferForTuple(relation, heaptup->t_len,InvalidBuffer, options, bistate, &vmbuffer, NULL)
    RelationPutHeapTuple(relation, buffer, heaptup)
    if PageIsAllVisible(BufferGetPage(buffer)):
        visibilitymap_clear(relation, ItemPointerGetBlockNumber(&(heaptup->t_self)), vmbuffer)
    MarkBufferDirty(buffer)

    if !(options & HEAP_INSERT_SKIP_WAL) && RelationNeedsWAL(relation):
        /* 如果是系统表 */
        if RelationIsAccessibleInLogicalDecoding(relation):
            log_heap_new_cid(relation, heaptup)
                ...
                XLogInsert(RM_HEAP2_ID, XLOG_HEAP2_NEW_CID)
        
        if ItemPointerGetOffsetNumber(&(heaptup->t_self)) == FirstOffsetNumber:
            info |= XLOG_HEAP_INIT_PAGE
            bufflags |= REGBUF_WILL_INIT
        
        XLogBeginInsert()
        XLogRegisterData((char *) &xlrec, SizeOfHeapInsert)
        XLogRegisterBuffer(0, buffer, REGBUF_STANDARD | bufflags)
        XLogRegisterBufData(0, (char *) &xlhdr, SizeOfHeapHeader)
        XLogRegisterBufData(0, heaptup->t_data + SizeofHeapTupleHeader, heaptup->t_len - SizeofHeapTupleHeader)
        XLogIncludeOrigin()

        recptr = XLogInsert(RM_HEAP_ID, info)
        PageSetLSN(page, recptr)
    
    CacheInvalidateHeapTuple(relation, heaptup, NULL)
    return HeapTupleGetOid(tup)
```

## init page时机
一共3处：
```c
heap_insert()
    ...
    Buffer buffer = RelationGetBufferForTuple()
    ...
    RelationPutHeapTuple(relation, buffer, heaptup)
    ...
    if ItemPointerGetOffsetNumber(&(heaptup->t_self)) == FirstOffsetNumber:
        info |= XLOG_HEAP_INIT_PAGE

heap_xlog_insert()
    if XLogRecGetInfo(record) & XLOG_HEAP_INIT_PAGE:
        buffer = XLogInitBufferForRedo(record, 0)
            XLogReadBufferForRedoExtended(RBM_ZERO_AND_LOCK)
                if XLogRecHasBlockImage(record, block_id):
                    *buf = XLogReadBufferExtended()
                    page = BufferGetPage(*buf)
                    RestoreBlockImage(record, block_id, page)
                    MarkBufferDirty(*buf)
                else:
                    *buf = XLogReadBufferExtended()
        page = BufferGetPage(buffer)
        PageInit(page, BufferGetPageSize(buffer), 0)
    else:
        XLogReadBufferForRedo(record, 0, &buffer)

    data = XLogRecGetBlockData(record, 0, &datalen)
    htup = &tbuf.hdr
    htup->t_infomask2 = xlhdr.t_infomask2
    htup ... = ...
    PageAddItem(page, (Item) htup, newlen, xlrec->offnum)
    MarkBufferDirty(buffer)

heap_multi_insert(HeapTuple *tuples, int ntuples)
    ...
    while (ndone < ntuples)
        buffer = RelationGetBufferForTuple()
        RelationPutHeapTuple()
        for (nthispage = 1; ndone + nthispage < ntuples; nthispage++)
            RelationPutHeapTuple()
            log_heap_new_cid(relation, heaptup)
        MarkBufferDirty(buffer)

        init = (ItemPointerGetOffsetNumber(&(heaptuples[ndone]->t_self)) == FirstOffsetNumber &&
			PageGetMaxOffsetNumber(page) == FirstOffsetNumber + nthispage - 1)
        if init:
            info |= XLOG_HEAP_INIT_PAGE
        ...


```