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

```c

```