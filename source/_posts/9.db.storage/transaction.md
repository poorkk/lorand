---
title: 存储引擎 执行流程
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---


```python
exec_simple_query()
    start_xact_command()
        StartTransactionCommand()
            StartTransaction()
                AtStart_Memory()
                AtStart_ResourceOwner()
                tid = GetNextLocalTransactionId()

    for parsetree:
        start_xact_command()

        if analyze need snapshot: # need snapshot: INSERT,DELETE,UPDATE,SELECT DECLARE,EXPLAIN,CREATE_TABLE_AS
            PushActiveSnapshot(GetTransactionSnapshot())
                if IsolationIsSerializable:
                    GetSerializableTransactionSnapshot()
                        GetSerializableTransactionSnapshotInt()
                            GetSnapshotData()
                else:
                    GetSnapshotData()
        # analyze, rewrite, plan
        PopActiveSnapshot()

        PortalStart()
            ChoosePortalStrategy()
            if PORTAL_ONE_SELECT:
                PushActiveSnapshot(GetTransactionSnapshot())
                ExecutorStart()
                PopActiveSnapshot()

        PortalRun()
            if PORTAL_ONE_SELECT:
                PortalRunSelect() # SELECT
                    PushActiveSnapshot()
                    ExecutorRun()
                        standard_ExecutorRun()
                            ExecutePlan()
                                ExecProcNode()
                    PopActiveSnapshot()
            elif PORTAL_MULTI_QUERY:
                PortalRunMulti()
                    if PlannedStmt: # INSERT,UPDATE,DELETE
                        PushActiveSnapshot(GetTransactionSnapshot())
                        ProcessQuery()
                            ExecutorStart()
                                standard_ExecutorStart()
                                    InitPlan()
                            ExecutorRun()
                            ExecutorFinish()
                                standard_ExecutorFinish()
                            ExecutorEnd()
                                standard_ExecutorEnd()
                                    UnregisterSnapshot()
                                        UnregisterSnapshotFromOwner()
                    else:
                        PortalRunUtility()
                            ProcessUtility()
                                standard_ProcessUtility()
                                    if T_TransactionStmt:
                                        if TRANS_STMT_BEGIN:
                                            BeginTransactionBlock()
                                        else if TRANS_STMT_COMMIT:
                                            EndTransactionBlock()
                                        # ...
                                    # else if Drop,TableSpace,Truncate,Copy,Grant,Database,Vacuum,Role,CheckPoint
                                    # else if Create,Alter,Index,Trig,

                    CommandCounterIncrement()
                        SnapshotSetCommandId()
    finish_xact_command()
        CommitTransactionCommand()

AbortCurrentTransaction()
```

```python
ExecScan()
    ExecScanFetch()
        SeqNext()
            heap_getnext() / 
                heapgettup()
                    heapgetpage()
                        ReadBufferExtended()
                    LockBuffer(BUFFER_LOCK_SHARE)
                    for tuple in page:
                        HeapTupleSatisfiesVisibility(scan->rs_snapshot)
                        CheckForSerializableConflictOut() // delete -> read
                    LockBuffer(BUFFER_LOCK_UNLOCK)

index_getnext()

ExecUpdate()
    heap_update()
        LockBuffer(BUFFER_LOCK_EXCLUSIVE)

```

```c
heap_...(SnapShot)

算子 -> heapam -> Buffer -> ...
```