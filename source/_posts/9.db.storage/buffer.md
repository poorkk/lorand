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
**写入数据**
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
