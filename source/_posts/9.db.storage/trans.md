---
title: 存储引擎 事务
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

```python
heap_insert(rel, tup, cid) # CommandId
    xid = get_cur_trans_id()

    heap_prepare_insert(rel, tup, xid, cid)
        heap_tup_set_xmin(tup, xid)
        heap_tup_set_cmin(tup, cid)
    
    buf = rel_get_buf_for_tup(rel, tup.len)
        blkno = get_page_with_free_space(rel, tlen)
            fsm_search(rel, tlen)
                fsm_search_avail(rel, tlen)
        read_buffer_bi(rel, blkno)
            read_buf(rel, blkno)
                read_buf_extend(rel, blkno)
                    read_buf_comm(rel, blkno)
                        bufhdr = buf_alloc(rel, blkno)
                        lwlock_acquire(bufhdr, LW_EXCLUSIVE)
        visibilitymap_pin(rel, vmbuf)
            vm_readbuf()
                vm_extend()
                read_buf_extend()
        lock_buffer(BUFFER_LOCK_EXCLUSIVE)
        ...
    
        needlock = not rel_is_local(rel)
        if needlock:
            LockRelationForExtension(rel, ExclusiveLock)
                LockAcquire(rel, ExclusiveLock)
                    LockAcquireExtended(rel, ExclusiveLock)

LockAcquireExtended(relid, mode)
    locallock = hash_search(LockMethodLocalHash, relid, enter)
    
```

```c
typedef struct {
    LOCKTAG tag;
    SHM_QUEUE proLocks; /* 持有锁的进程：PROCLOCK  */
    PROC_QUEUE waitProcs; /* PGPROC */

    int granted[MAX_LOCKMODES]; /* 锁上加锁次数，会同时持有多种锁 */
} LOCK;

typedef struct {
    PROCLOCKTAG tag;
    PGPROC *groupLeader;
    SHM_QUEUE lockLink; /* LOCK */
    SHM_QUEUE proLink; /* PGPROC */
} PROCLOCK;
```

每个后端进程维护正持有的锁和等待加的锁。
```c
typedef struct {
    LOCALLOCKTAG tag;

    LOCK lock;
    PROCLOCK *proclock;
    int64 nLocks; /* 持有锁数量 */

} LOCALLOCK;
```

加锁：LockAcquire()
1. 查找进程持有锁，如果存在，返回锁，granted++
2. 通过fastpath找锁
3. 在哈希表中加锁：先创建LOCK，PROLOCK
4. if 加锁与等锁冲突：等待，if 加锁失败：等待
5. if dontwait: 返回加锁失败，否则，进入等待队列，进程休眠

放锁：
1. 查找进程持有锁，如果存在，if --ResourceOwner = 0: 释放锁。granted--
2. 从fastpath找锁
3. 从哈希表找锁
4. 释放锁，唤醒其他等锁进程

| 锁 |  取值 | 操作 |
|-|-|-|
AccessShareLock | 1 | SELECT
RowShareLock | 2 | SELECT FOR UPDATE/SHARE
RowExclusiveLock | 3 | INSERT/UPDATE/DELETE
ShareUpdateExclusiveLock | 4 | VACUUM/ANALYZE/CRATE INDEX CONCURRENTLY
ShareLock | 5 | CREATE INDEX
ShareRowExclusiveLock | 6 | row share
ExclusiveLock | 7 | block ROW SHARE/SELECT FOR UPDATE
AccessExclusiveLock | 8 | ALTER TABLE/DROP TABLE/VACUUM FULL

```python
- heap_open(relid lockmode) # not index
- relation_open(relid, lockmode) # any rel
    LockRelationOid(relid, lockmode)
        LockAcquire(tag, lockmode)
    RelationIdGetRelation(relid)
        if RelationIdCacheLookup(relationId, rd) -> hash_search(RelationIdCache)
            RelationIncrementReferenceCount(rel)
        else RelationBuildDesc(relid)
            RelationIncrementReferenceCount(rel)
- heap_close(relid, lockmode)
- relation_close(rel, lockmode)
    RelationClose(rel)
        RelationDecrementReferenceCount()
        UnlockRelationId(lockmode)

heap_insert(rel, tup, cid)
    xid = GetCurrentTransactionId()
    tup = heap_prepare_insert() # set [ insert xid | delete xid | cmd id | ... ]
    buf = RelationGetBufferForTuple(rel, tup.len)
        LockRelationForExtension(rel, ExclusiveLock)
        ReadBuffer(rel, blkno) # LWLockAcquire(bufhdr, LW_EXCLUSIVE), LockBuffer(buffer, BUFFER_LOCK_EXCLUSIVE)
        UnlockRelationForExtension(rel, ExclusiveLock)
    RelationPutHeapTuple(rel, buf, tup) # PageAddItem
    /* xlog */
    UnlockReleaseBuffer(buf)
        LockBuffer(buf, BUFFER_LOCK_UNLOCK)
        ReleaseBuffer(buf)
heap_delete(rel, tid, cid, snapshot)
    xid = GetCurrentTransactionId()
    buf = ReadBuffer(rel, ItemPointerGetBlockNumber(tid))
    LockBuffer(buf, BUFFER_LOCK_EXCLUSIVE)
    ... 复杂
    heap_acquire_tuplock(rel, tup, LockTupleExclusive, LockWaitBlock)
    ...
    UnlockTupleTuplock(rel, tup, LockTupleExclusive)
heap_update(rel, okdtid, newtup, cid, snapshot, lockmode)
```

```c
relation锁：

buffer锁： call LWLockAcquire(LW_SHARED / LW_EXCLUSIVE)
#define BUFFER_LOCK_UNLOCK		0
#define BUFFER_LOCK_SHARE		1
#define BUFFER_LOCK_EXCLUSIVE	2

tuple锁：
typedef enum {
    LockTupleKeyShare, /* SELECT FOR KEY SHARE */
    LockTupleShare, /* SELECT FOR SHARE */
    LockTupleNoKeyExclusive, /* SELECT FOR NO KEY UPDATE, and UPDATEs that don't modify key columns */
    LockTupleExclusive, /* SELECT FOR UPDATE, UPDATEs that modify key columns, and DELETE */
} LockTupleMode; 映射到reguler锁 LockMode
{
    {AccessShareLock, MultiXactStatusForKeyShare, -1},
    {RowShareLock, MultiXactStatusForShare, -1},
    {ExclusiveLock, MultiXactStatusForNoKeyUpdate, MultiXactStatusNoKeyUpdate,},
    {AccessExclusiveLock, MultiXactStatusForUpdate, MultiXactStatusUpdate}
}

LockTupleTuplock(rel, tid, lockmode)
    LockTuple(rel, tup, tupleLockExtraInfo[lockmode])
        LockAcquire(tag, lockmode)

typedef enum
{
	MultiXactStatusForKeyShare = 0x00,
	MultiXactStatusForShare = 0x01,
	MultiXactStatusForNoKeyUpdate = 0x02,
	MultiXactStatusForUpdate = 0x03,
	/* an update that doesn't touch "key" columns */
	MultiXactStatusNoKeyUpdate = 0x04,
	/* other updates, and delete */
	MultiXactStatusUpdate = 0x05
} MultiXactStatus;
```