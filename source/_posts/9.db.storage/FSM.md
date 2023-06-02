---
title: 存储引擎 FSM
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

模块对外接口
```c
Size GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk);

BlockNumber GetPageWithFreeSpace(Relation rel, Size spaceNeeded);
void RecordPageWithFreeSpace(Relation rel, BlockNumber heapBlk, Size spaceAvail);
void XLogRecordPageWithFreeSpace(RelFileNode rnode, BlockNumber heapBlk, Size spaceAvail);

void FreeSpaceMapVacuum(Relation rel);
void UpdateFreeSpaceMap(Relation rel, BlockNumber startBlkNum, BlockNumber endBlkNum, Size freespace);
```

FSM文件的格式：
```c
typedef struct {
    int fp_next_slot;
    uint8 fp_nodes[FLEXIBLE_ARRAY_MEMBER];
} FSMPage;

/* 初始化fsm page */
fsm_extend(Relation rel) {
    PageInit(page)
    fsm_nblocks_now = smgrnblocks(rel->rd_smgr, FSM_FORKNUM)
    smgrextend(rel->rd_smgr, FSM_FORKNUM, fsm_nblocks_now)
}

/* 读取fsm page */
Buffer fsm_readbuf(Relation rel, FSMAddress addr)
    
```

模块内部实现：
```c
Size GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk)
    uint16 slog;
    FSMAddress	addr = fsm_get_location(heapBlk, &slot)
    Buffer buf = fsm_readbuf(rel, addr, false)
    unint8 cat = fsm_get_avail(BufferGetPage(buf), slot)
    return fsm_space_cat_to_avail(cat)
```