---
title: 存储引擎 缓冲区管理
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

# 1 概述
## 1.1 buffer池模型
模型：
```c
    哈希表
SharedBufHash                Buffer
+-----+                        +-------+
| 1   |                     | 1     |
| 2   |                     | 2     |
| 3   |                    | 3     |
| ... |                      | 4     |
| 128 |                       | ...   |
+-----+                      
                        | 1000  |
                        +-------+
```

## 1.2 buffer池接口
缓冲区接口：
```c
/* buffer池管理 */
void InitBufferPool(void);
void InitBufferPoolAccess(void);
void InitBufferPoolBackend(void);

/* buffer管理 */
Buffer ReadBuffer(Relation reln, BlockNumber blockNum);
Buffer ReadBufferExtended(Relation reln, ForkNumber forkNum, BlockNumber blockNum, ReadBufferMode mode, BufferAccessStrategy strategy);

void LockBuffer(Buffer buffer, int mode);
void UnlockBuffers(void);
void MarkBufferDirty(Buffer buffer);
void FlushOneBuffer(Buffer buffer);
void ReleaseBuffer(Buffer buffer);
```

进程接口：
```c
void CheckPointBuffers(int flags);
bool BgBufferSync(void);
```

## 1.3 buffer调用
**插入数据**
```c
heap_insert()
    RelationGetBufferForTuple()
        /* 已有page，查找空闲的page */
        GetPageWithFreeSpace()
        
        /* 读取page，放入缓冲池 */
        ReadBufferBI()
            ReadBuffer() /* buffer接口 */
                ReadBufferExtended()
                    ReadBuffer_common()
                        BufferAlloc() /* 遍历buffer池，查找 */
```

**查找页面**
```c
BufferDesc BufferAlloc()
    BufTableHashCode(newtag)
    buf_id = BufTableLookup()
    if buf_id:
        buf_desc = GetBufferDescriptor(buf_id)
        PinBuffer(buf_desc)
        return buf_desc;

    for (;;) {
        buf_desc = StrategyGetBuffer()
        if buf_desc dirty :
            FlushBuffer(buf_desc)
        if buf_desc tab_valid: /* 旧buf有数据，但旧buf可用 */
            /* partition lock ? */
        buf_id = BufTableInsert(newtag, buf_desc->buf_id) /* 可能失败，多进程一起申请同一page */
        if buf_id:
            return buf_desc
        BufTableDelete(newtag)
    }

    BufTableDelete(oldtag)
```

# 2 实现
## 2.1 ReadBuffer
```c
Buffer ReadBuffer(Relation reln, BlockNumber blockNum)
    Buffer buf = ReadBufferExtended(reln, MAIN_FORKNUM, blockNum, RBM_NORMAL, NULL) /* 读场景 strategy = NULL */
        Buffer buf = ReadBuffer_common(SMgrRelation smgr, char relpersistence, ForkNumber forkNum, BlockNumber blockNum, ReadBufferMode mode, BufferAccessStrategy strategy, bool *hit) {
            /* 1. 根据表类型判断，使用LocalBuffer还是共享Buffer */
            bool isLocalBuf = SmgrIsTemp(smgr)
            if isLocalBuf:
                LocalBufferAlloc()
            else:
                /* 选择1个buffer槽，{fornNum, BlockNum}等信息写入buffer槽 */
                BufferDesc *bufHdr = BufferAlloc(smgr, forkNum, blockNum, strategy, &found) {
                    uint32 newHash = BufTableHashCode({smgr->smgr_rnode.node, forkNum, blockNum})
                    /* 2. 根据blockNum等信息，从哈希表中查找buf_id */
                    int buf_id = BufTableLookup(&newTag, newHash)
                    if buf_id >= 0: /* 如果找到（已在buffer池中） */
                        /* 3.1 根据buf_id，查找BufferDesc */
                        BufferDesc *buf = GetBufferDescriptor(buf_id)
                        /* 3.2 根据BufferDesc，引用计数++，使用计数++ */
                        PinBuffer(BufferDesc *buf)
                            buf->refcount++
                            buf->usage_count++
                        /* 异常处理 */
                        return buf

                    for (;;) /* 如果没找到 */
                        BufferDesc *buf = StrategyGetBuffer(strategy)
                            BufferDesc *buf = GetBufferFromRing()
                                bgwprocno = StrategyControl->bgwprocno
                                /* 4.1 唤醒bg writer */
                                SetLatch(&ProcGlobal->allProcs[bgwprocno].procLatch)
                                /* 4.2 检查是否有空闲Buffer */
                                if StrategyControl->firstFreeBuffer >= 0:
                                    buf = GetBufferDescriptor(StrategyControl->firstFreeBuffer)
                                    StrategyControl->firstFreeBuffer = buf->freeNext
                                    /* 4.3 TODO */
                        /* 4.4 根据BufferDesc，引用计数++，使用计数++ */
                        if buf->flags & BM_DIRTY:
                            /* 4.5 将选中的Buffer落盘 */
                            FlushBuffer(buf)
                        if buf->flags & BM_TAG_VALID:
                            oldTag = buf->tag
                            oldHash = BufTableHashCode(&oldTag)
                        /* 4.6 将hash表中受害者Buffer对应的位置替换为即将写入的的BufferTag，-1表示成功，否则返回已存在的冲突的buffer id */
                        buf_id = BufTableInsert(&newTag, newHash, buf->buf_id)
                        /* 4.7 如果其他进程已经将新的buffer写入哈希表，直接返回即可 */
                        if buf_id >= 0:
                            PinBuffer(buf)
                            return buf
                        /* 4.8 替换映射成功 */
                        LockBufHdr(buf)
                        oldFlags = buf->flags
                        if buf->refcount == 1 && !(oldFlags & BM_DIRTY): /* 4.9 替换映射成功，且受害者buffer没有被重新写入数据 */
                            break;
                        /* 4.10 受害者buffer又被重新写入新数据，需要重新进入循环选择新的受害者 */
                        BufTableDelete(&newTag, newHash)
                        UnpinBuffer(buf, true)
                
                    /* 将受害者buffer对用的信息替换为新page，并将buffer槽标记为VM_INVALID */
                    buf->tag = newTag
                    buf->flags &= ~(BM_VALID | BM_DIRTY | BM_JUST_DIRTIED | BM_CHECKPOINT_NEEDED | BM_IO_ERROR | BM_PERMANENT)
                    buf->usage_count = 1
                    if oldFlags & BM_TAG_VALID:
                        BufTableDelete(&oldTag, oldHash)
                    return buf
                }

                /* 5. 如果page已在Buffer池中，直接返回即可 */
                if found:
                    *hit = true
                    return BufferDescriptorGetBuffer(bufHdr)
                
                /* 6. 从磁盘读取page，并且直接读如buffer槽中 */
                bufBlock = isLocalBuf ? LocalBufHdrGetBlock(bufHdr) : BufHdrGetBlock(bufHdr)
                smgrread(smgr, forkNum, blockNum, bufBlock)

                if !PageIsVerified(bufBlock):
                    ereport ...
                
                /* 7. 在bufHdr中将buffer设置未BM_VALID */
                TerminateBufferIO(bufHdr, false, BM_VALID)

                return BufferDescriptorGetBuffer(bufHdr)
        }                            
```