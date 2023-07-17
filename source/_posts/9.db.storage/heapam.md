---
title: 存储引擎 heapam
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

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