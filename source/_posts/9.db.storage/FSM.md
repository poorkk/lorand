---
title: 存储引擎 FSM
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

# 1 概述
## 1.1 思想
**fsm结构**
- 用数组表示完全二叉树
```bash
      | 8 |
    | 4 | 8 |
| 4 | 2 | 1 |  8 |
```

## 1.2 FSM涉及
**物理页号**
```bash
            +-------+-------+-------+-------+-------+
FSMFile     | 8k     | 8k   | 8k    | 8k    | ...   |
            +-------+-------+-------+-------+-------+
FSMBlock    | 0     | 1     | 2     | 3     | ...   |
            +-------+-------+-------+-------+-------+
```

**逻辑页号**
第2层：1个FSMBlock
第1层：4069个FSMBlock
第0层：4069 * 4069个FSMBlock，每个FSM可映射4069个DataBlock
FSMAddress: [层次，页号]，页号即FSMBlock号
每个表最大容量是32T，只需3层也页面即可管理整个表的空闲空间
```bash
# [FSMAddress.level, FSMAddress.logpageno, FSMBlockNum（1个page即1个FSMBlock）]
                                             [2,0,0]
                                                |
                    +---------------------------+---------------------------+
                    |                           |                           |
                [1,0,1]                 [1,1,1*4070+1]  ...  [1,4069,4069*4070+1]  本层共4069个FSMBlock
                    |
         +----------+------------+
         |          |            |
    [2,0,1+1]  [2,1,1+2] ... [2,2,1+4069]     ......                                本层共4069*4069个FSMBlock
         |
（映射DataBlock 0 ~ 4065）
```

```c
/* FSMBlock 逻辑地址 */
typedef struct {
    int level;
    int logpageno; /* 当前层内的块号 */
} FSMAddress;

typedef struct {
    int fp_next_slot;
    uint8 fp_nodes[FLEXIBLE_ARRAY_MEMBER];
} FSMPage;

/* 每个FSMBlock中的节点数 8192 - 24 - 4 = 8164 */
#define NodesPerPage (BLCKSZ - MAXALIGN(SizeOfPageHeaderData) - offsetof(FSMPageData, fp_nodes))
/* 每个FSMBlock中的非叶子节点数 8192/2 - 1 = 4095 */
#define NonLeafNodesPerPage (BLCKSZ / 2 - 1)
/* 每个FSMBlock中的叶子节点数 8164 - 4095 = 4069 */
#define LeafNodesPerPage (NodesPerPage - NonLeafNodesPerPage)
/* 每个FSMBlock中slot数 4069 */
#define SlotsPerFSMPage LeafNodesPerPage

/* 根节点所在层号 0 */
#define FSM_ROOT_LEVEL  (FSM_TREE_DEPTH - 1)
/* 叶子节点所在的层号 0 */
#define FSM_BOTTOM_LEVEL 0
```

**叶子节点取值**
用1个字节表示对应DataBlock的空闲空间级别
每个级别表示32字节空闲空间
| 级别 | 对应data page的空闲空间 |
|-|-|
0 | 0 ~ 31
1 | 31 ~ 63
3 | 64 ~ 92
... | ...
255 | 8164 ~ 8192

```c
/* FSM空闲空间级别 */
#define FSM_CATEGORIES  256 
/* FSM空闲空间级别的步长 8192/256 = 32 */
#define FSM_CAT_STEP    (BLCKSZ / FSM_CATEGORIES)
```

# 2 函数
## 2.1 FSMBlock定位
```c
FSMAddress fsm_get_location(BlockNumber heapblk, uint16 *slot)
    FSMAddress addr
    addr.level = FSM_BOTTOM_LEVEL
    addr.logpageno = heapblk / SlotsPerFSMPage
    slot = heapblk % SlotsPerFSMPage
    return addr
BlockNumber fsm_get_heap_blk(FSMAddress addr, uint16 slot)
    Assert(addr.level == FSM_BOTTOM_LEVEL)
    return ((unsigned int)addr.logpageno) * SlotsPerFSMPage + slot
BlockNumber fsm_logical_to_physical(FSMAddress addr)
    int leafno = addr.logpageno
    for (l = 0; l < addr.level; l++)
        leafno *= SlotsPerFSMPage /* e.g [1,1] = 1 * 4069 */
    BlockNumber pages = 0
    for (l = 0; l < FSM_TREE_DEPTH; l++)
        pages += leafno + 1 /* e.g [1,1] -> 4069 -> 4069 + 1 */
        leafno /= SlotsPerFSMPage
    pages -= addr.level
    return pages - 1
FSMAddress fsm_get_child(FSMAddress parent, uint16 slot) /* slog为块内地址 */
    FSMAddress child
    child.level = parent.level - 1
    child.logpageno = parent.logpageno * SlotsPerFSMPage + slot
    return child
FSMAddress fsm_get_parent(FSMAddress child, uint16 *slot)
    SMAddress parent
    parent.level = child.level + 1
    parent.logpageno = child.logpageno / SlotsPerFSMPage
    *slot = child.logpageno % SlotsPerFSMPage
```

## 2.2 FSMBlock读写
```c
/* 初始化fsm page */
fsm_extend(Relation rel)
    PageInit(page)
    fsm_nblocks_now = smgrnblocks(rel->rd_smgr, FSM_FORKNUM)
    smgrextend(rel->rd_smgr, FSM_FORKNUM, fsm_nblocks_now)

/* 读取fsm page */
Buffer fsm_readbuf(Relation rel, FSMAddress addr)
    lockNumber blkno = fsm_logical_to_physical(addr)
    buf = ReadBufferExtended(rel, FSM_FORKNUM)
    if PageIsNew(BufferGetPage(buf))
        PageInit(BufferGetPage(buf))
    return buf
```

## 2.3 FSMBlock计算
```c
int fsm_search_avail(Buffer buf, uint8 min_cat, bool advancenext, bool exclusive_lock_held);
uint8 fsm_get_avail(Page page, int slot);
uint8 fsm_get_max_avail(Page page);
bool fsm_set_avail(Page page, int slot, uint8 value);
bool fsm_truncate_avail(Page page, int nslots);
bool fsm_rebuild_page(Page page);

/* FSMBlock内是大根对 */
int fsm_search_avail(Buffer buf, uint8 minvalue, bool advancenext, bool exclusive_lock_held)
restart:
    FSMPage	fsmpage = (FSMPage)PageGetContents(BufferGetPage(buf))
    int target = fsmpage->fp_next_slot
    if target < 0 || target >= LeafNodesPerPage:
        target = 0
    target += NonLeafNodesPerPage

    int nodeno = target
    while nodeno > 0:
        if fsmpage->fp_nodes[nodeno] >= minvalue:
            break;
        nodeno = parentof(rightneighbor(nodeno))
    
    while nodeno < NonLeafNodesPerPage:
        int childnodeno = leftchild(nodeno)
        if childnodeno < NodesPerPage && fsmpage->fp_nodes[childnodeno] >= minvalue:
            nodeno = childnodeno
            continue
        childnodeno++
        if childnodeno < NodesPerPage && fsmpage->fp_nodes[childnodeno] >= minvalue:
            nodeno = childnodeno
        else:
            RelFileNode rnode
            ForkNumber forknum
            BlockNumber blknum
            BufferGetTag(buf, &rnode, &forknum, &blknum)
            LockBuffer(buf, BUFFER_LOCK_EXCLUSIVE)
            fsm_rebuild_page(page)
            MarkBufferDirtyHint(buf, false)
            goto restart

    lot = nodeno - NonLeafNodesPerPage
    fsmpage->fp_next_slot = slot + (advancenext ? 1 : 0)
    return slot
```

模块对外接口
```c
Size GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk);
BlockNumber GetPageWithFreeSpace(Relation rel, Size spaceNeeded);
void RecordPageWithFreeSpace(Relation rel, BlockNumber heapBlk, Size spaceAvail);
void XLogRecordPageWithFreeSpace(RelFileNode rnode, BlockNumber heapBlk, Size spaceAvail);
void FreeSpaceMapVacuum(Relation rel);
void UpdateFreeSpaceMap(Relation rel, BlockNumber startBlkNum, BlockNumber endBlkNum, Size freespace);

Size GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk)
    uint16 slog;
    FSMAddress	addr = fsm_get_location(heapBlk, &slot)
    Buffer buf = fsm_readbuf(rel, addr, false)
    unint8 cat = fsm_get_avail(BufferGetPage(buf), slot)
    return fsm_space_cat_to_avail(cat)
```

- https://www.jianshu.com/p/4064dbb72414