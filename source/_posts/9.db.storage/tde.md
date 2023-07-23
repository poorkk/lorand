# 1 概述
## 1.1 模块
```bash
heapam -> buffer -> smgr -> page
```

# 2 调用关系
## 2.1 初始化数据页
### page_init
### image
## 2.2  写入数据页
### smgrextend
调用栈：
```c
smgrextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, char *buffer)
    copy_relation_data()
        ATExecSetTableSpace() /* ALTER TABLE ... TABLESPACE */
            ATRewriteTables()
                ATController()
                    AlterTable()
                    AlterTableInternal()
    raw_heap_insert()
        rewrite_heap_tuple()
            reform_and_rewrite_tuple()
                copy_heap_data()
                    rebuild_relation()
                        cluster_rel()
                            cluster() /* case T_ClusterStmt */
                                standard_ProcessUtility()
                            vacuum_rel()
                                vacuum()
                                    ExecVacuum() /* case T_VacuumStmt  */
                                        standard_ProcessUtility()
                                    autovacuum_do_vac_analyze()
                                        do_autovacuum()
                                            AutoVacWorkerMain()
                                                StartAutoVacWorker() /* TODO */
                                                SubPostmasterMain() /* --forkavworker */
        end_heap_rewrite()
            copy_heap_data()
                /* repeat */
    end_heap_rewrite()
        /* repeat */
    ReadBuffer_common()
        ReadBufferExtended() /* 48 call */
            get_raw_page_internal()
            pg_prewarm()
            statapprox_heap()
            pgstatindex_impl()
            /* TODO */
        ReadBufferWithoutRelcache()
            XLogReadBufferExtended()
                XLogReadBufferForRedoExtended()
                    XLogReadBufferForRedo() /* 58 call */
                    XLogInitBufferForRedo() /* 33 call */
                    heap_xlog_clean()
                    heap_xlog_visible()
                    btree_xlog_vacuum()
                XLogRecordPageWithFreeSpace() /* 剪枝：FSM_FORKNUM */
                    heap_xlog_clean()
                        /* repeat */
                    heap_xlog_insert()
                    heap_xlog_multi_insert()
                    heap_xlog_update()
                btree_xlog_vacuum()
                btree_xlog_delete_get_latestRemovedXid()
    _hash_alloc_buckets()
        _hash_expandtable()
            _hash_doinsert()
                hashbuildCallback()
                hashinsert()
                _h_indexbuild()
    _bt_blwritepage()
    fsm_extend()
    vm_extend()
```
### smgrwrite
调用栈
```bash
smgrwrite(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, char *buffer)
    FlushBuffer()
        BufferAlloc()
            /* START FROM HERE */
        SyncOneBuffer()
        FlushOneBuffer()
        FlushRelationBuffers()
        FlushDatabaseBuffers()
    FlushRelationBuffers()
    LocalBufferAlloc()
    btbuildempty()
    _bt_blwritepage()
    spgbuildempty()
```
## 2.3 读取数据页
## 2.4 记录数据

# 2 记录
xlog记录类型：（仅针对用户表）
1. 1条tuple
    heap_insert() : RM_HEAP_ID, XLOG_HEAP_INSERT | XLOG_HEAP_INIT_PAGE 
    heap_delete() : RM_HEAP_ID, XLOG_HEAP_DELETE
    heap_update() : RM_HEAP_ID, XLOG_HEAP_LOCK
    heap_lock_tuple() : RM_HEAP_ID, XLOG_HEAP_LOCK
    heap_lock_updated_tuple_rec() : RM_HEAP2_ID, XLOG_HEAP2_LOCK_UPDATED
    heap_finish_speculative() : RM_HEAP_ID, XLOG_HEAP_CONFIRM
    heap_abort_speculative() : ...
    heap_inplace_update : RM_HEAP_ID, XLOG_HEAP_INPLACE

2. 多条tuple
    heap_multi_insert() : RM_HEAP2_ID, XLOG_HEAP2_MULTI_INSERT | XLOG_HEAP_INIT_PAGE

3. 1页page

1. 数据写入时机：
    单写：
    1. INSERT
    批量写：
    2. ALTER ... TABLESPACE
    3. VACUUM FULL

log_newpage场景：
1. vacuum full: 创建新文件，拷贝旧文件