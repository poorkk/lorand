---
title: 存储引擎 9.5 WAL机制
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

# 1 xlog格式
## 1.1 wal 背景
- 功能需求：
    - 执行事务/事务块，避免故障导致数据丢失，产生的数据写入磁盘才算事务成功
- 性能问题：
    - 执行事务/事务块，比如一条INSERT，除了要写数据文件，更新fsm文件，vm文件，索引文件，更新事务提交文件等
    

## 1.1 格式
通用的xlog记录格式 (pg 9.5+)
```bash
+---------------------------------+
| XLogRecord                      |
+---------------------------------+ 设置对应flag则有BlockHeader，否则没有
| XlogRecordBlockHeader           | 设置flag
|   XLogRecordBlockImageHeader    | - if BKPBLOCK_HAS_IMAGE
|   XLogRecordBlockCompressHeader | - if BKPBLOCK_HAS_HOLE & BKPIMAGE_IS_COMPRESSED
|   RelFileNode                   | - if not BKPBLOCK_SAME_REL
| XlogRecordBlockHeader           |
| ...                             |
+---------------------------------+
| XlogRecordDataHeader            | Short/Long (判断条件：长度 < 256 bytes)
+---------------------------------+
| block data                      |
| block data                      |
| ...                             |
| data                            |
+---------------------------------+
```

各字段结构体定义
```c
/* 1 */
typedef struct {
    uint32          xl_tot_len; /* 本条xlgo记录全部长度 */
    TransactionId   xl_xid;
    XLogRecPtr      xl_prev;    /* 上一条xlog记录的位置 */
    uint8		    xl_info;    /* 资管管理器使用，判断调用哪个重放函数 */
    RmgrId		    xl_rmid;    /* 资源管理器 */
    pg_crc32c	    xl_crc;		/* 本条xlog记录的crc */
} XLogRecord;

/* 2 */
typedef struct {
    uint8		id;	         /* block reference id */
    uint8		fork_flags;	
    uint16		data_length; /* 数据实际长度，不包括image */

	/* If BKPBLOCK_HAS_IMAGE, an XLogRecordBlockImageHeader struct follows */
	/* If BKPBLOCK_SAME_REL is not set, a RelFileNode follows */
	/* BlockNumber follows */
} XLogRecordBlockHeader;

/* 其中，根据fork_flags，判断是否有其他BlockHeader */
#define BKPBLOCK_HAS_IMAGE	0x10	/* block data is an XLogRecordBlockImage */
#define BKPBLOCK_HAS_DATA	0x20
#define BKPBLOCK_WILL_INIT	0x40	/* redo will re-init the page */
#define BKPBLOCK_SAME_REL	0x80	/* RelFileNode omitted, same as previous */

typedef struct {
	uint16		length;			/* number of page image bytes */
	uint16		hole_offset;	/* number of bytes before "hole" */
	uint8		bimg_info;		/* flag bits, see below */
} XLogRecordBlockImageHeader;

typedef struct {
    uint16		hole_length; /* number of bytes in "hole" : page中没有数据的tup就是孔 */
} XLogRecordBlockCompressHeader;

/* 3 */
typedef struct {
	uint8		id;				/* XLR_BLOCK_ID_DATA_SHORT */
	uint8		data_length;	/* number of payload bytes */
} XLogRecordDataHeaderShort;

typedef struct {
	uint8		id;				/* XLR_BLOCK_ID_DATA_LONG */
} XLogRecordDataHeaderLong;

/* 其中，id的取值包括 */
#define XLR_MAX_BLOCK_ID			32
#define XLR_BLOCK_ID_DATA_SHORT		255
#define XLR_BLOCK_ID_DATA_LONG		254
#define XLR_BLOCK_ID_ORIGIN			253

```

## 1.2 插入顺序
```c
XLogBeginInsert() /* 检查受否在故障恢复 */
XLogRegisterData(data, datasz) /*  */
XLogRecPtr XLogInsert(RmgrId rmid, uint8 info)
XLogFlush(XLogRecPtr record)
```

heap_insert
```c
/* 结构体：输入 */
typedef struct {
    uint32 t_len;
    ItemPointerData t_self;
    Oid t_tableOid;
    HeapTupleHeader t_data;
} HeapTuple;

/* 输出 */
typedef struct {
    OffsetNumber offnum;
    uint8        flags;
} xl_heap_insert;

typedef struct {
    uint16 t_infomask2;
    uint16 t_infomask
    uint8  t_hoff;
} xl_heap_header;

/* 执行流程 */
HeapTuple tup;

xl_heap_insert xlrec;
xl_heap_header xlhdr;
uint8 info = XLOG_HEAP_INSERT;
int bufflags = 0;
XLogRecPtr recptr;

xlrec.offnum = tup.t_self.offset;
xlrec.flag = 0; /* if page XLH_INSERT_ALL_VISIBLE_CLEARED XLH_INSERT_IS_SPECULATIVE */

xlhdr.t_infomask2 = tup->t_data->t_infomask2;
xlhdr.t_infomask = tup->t_data->t_infomask;
xlhdr.t_hoff = tup->t_data->t_hoff;

XLogBeginInsert()
/* put tup place */
XLogRegisterData(&xlrec, sizeof(xlrec));
/* put buffer where tup come from */
XLogRegisterBuffer(0, buffer, bufflags | REGBUF_STANDARD);
/* put tup */
XLogRegisterBufData(0, &xlhdr, sizeof(xlhdr));
XLogRegisterBufData(0, tup->t_data + sizeof(HeapTupleHeader), tup->t_len - sizeof(HeapTupleHeader));
XLogIncludeOrigin();
recptr = XLogInsert(RM_HEAP_ID, info);

PageSetLSN(page, recptr);
```

内部实现：
```c
typedef struct XlogRecData {
    struct XlogRecData *next;
    char *data;
    uint32 len;
} XLogRecData;

/* 进程级全局变量：

+-------------+-------------+-------------+
| XLogRecData | XLogRecData | ...         |
+-------------+-------------+-------------+
              ^ last
*/

XLogRecData *rdatas; /* XLogRecData 数组, len = 20 */
int num_rdatas;
int max_rdatas;

XLogRecData *mainrdata_head;
XLogRecData *mainrdata_last;
uint32 mainrdata_len; /* chain中的数据总长度 */

XLogRegisterData(data, len)
    XLogRecData *rdata = rdatas[num_rdatas++]; /* global arr */
    rdata->data = data;
    rdata->len = len;

    mainrdata_last->next = rdata;
    mainrdata_last = rdata;
    mainrdata_len += len;

typedef struct {
    bool        in_use;
    uint8       flags;
    RelFileNode rnode;
    ForkNumber	forkno;
    BlockNumber block;
    Page		page;

    uint32		rdata_len; /* total length of data in rdata chain */
    XLogRecData *rdata_head;
    XLogRecData *rdata_tail;
    XLogRecData bkp_rdatas[2]; /* temporary rdatas */

    char compressed_page[8192 + 4];
} registered_buffer;

/* 进程级全局变量：

+-------------------+-------------------+-------------+
| registered_buffer | registered_buffer | ...         |
+-------------------+-------------------+-------------+
|-> rdata -> rdata  |-> ...
*/

registered_buffer *registered_buffers; /* len = 4 + 1, may realloc */
int max_registered_buffers;
int max_registered_block_id = 4 + 1;

XLogRegisterBuffer(block_id, buffer, flags)
    /* block_id = 0 */
    max_registered_block_id += (block_id >= max_registered_block_id) ? 1 : 0;
    
    registered_buffer regbuf = &registered_buffers[block_id]; /* global arr */
    BufferGetTag(buffer, &regbuf->rnode, &regbuf->forkno, &regbuf->block);
	regbuf->page = BufferGetPage(buffer);
	regbuf->flags = flags;

	regbuf->rdata_tail = (XLogRecData *) &regbuf->rdata_head;
	regbuf->rdata_len = 0;
    regbuf->in_use = true;

XLogRegisterBufData(block_id, data, len)
    XLogRecData *rdata = &rdatas[num_rdatas++]; /* global arr */
    rdata->data = data;
    rdata->len = len;
    
    registered_buffer *regbuf = &registered_buffers[block_id]; /* global arr */
    regbuf->rdata_tail->next = rdata;
    regbuf->rdata_tail = rdata;
    regbuf->rdata_len += len;

typedef uint64 XLogRecPtr;

/* 进程全局变量 */
XLogRecPtr RedoRecPtr;
bool doPageWrites;

XLogInsert(rmid, info)
    XLogRecPtr	fpw_lsn;
    XLogRecData rdt = XLogRecordAssemble(rmid, info, RedoRecPtr, doPageWrites, &fpw_lsn)
    XLogRecPtr EndPos = XLogInsertRecord(rdt, fpw_lsn);
    return EndPos; /* 即 page lsn */

XLogRecData hdr_rdt; /* 把所有buffer里和buffer外的所有list merge在一起 */
char *hdr_scratch = NULL; /* 把所有buffer里和buffer外的所有数据拼接在一起 */

XLogRecData *XLogRecordAssemble(rmid, info, RedoRecPtr, doPageWrites, fpw_lsn)
    char	   *scratch = hdr_scratch;
    XLogRecord *rechdr = (XLogRecord)scratch;
    scratch += sizeof(XLogRecord);

    hdr_rdt.next = NULL;
	XLogRecData *rdt_datas_last = &hdr_rdt;
	hdr_rdt.data = hdr_scratch;

    /* 处理buffer */
    registered_buffer *prev_regbuf = NULL;
    for (block_id = 0; block_id < max_registered_block_id; block_id++) {
        registered_buffer *regbuf = &registered_buffers[block_id]
        
        if !regbuf->in_use:
            continue;

        if regbuf->flags & REGBUF_FORCE_IMAGE：
            needs_backup = true
        else：
            XLogRecPtr	page_lsn = PageGetLSN(regbuf->page);
            if *fpw_lsn == InvalidXLogRecPtr || page_lsn < *fpw_lsn:
                *fpw_lsn = page_lsn;

        if regbuf->rdata_len == 0:
            needs_data = false;
        
        XLogRecordBlockHeader bkpb;
		bkpb.id = block_id;
		bkpb.fork_flags = regbuf->forkno;
		bkpb.data_length = 0;

        if regbuf->flags & REGBUF_WILL_INIT:
            bkpb.fork_flags |= BKPBLOCK_WILL_INIT;
        
        if needs_backup:
            TODO
        
        if needs_data:
			bkpb.fork_flags |= BKPBLOCK_HAS_DATA;
			bkpb.data_length = regbuf->rdata_len;
			total_len += regbuf->rdata_len;

			rdt_datas_last->next = regbuf->rdata_head;
			rdt_datas_last = regbuf->rdata_tail;

        if prev_regbuf && regbuf->rnode == prev_regbuf->rnode:
            samerel = true;
            bkpb.fork_flags |= BKPBLOCK_SAME_REL;
        else:
            samerel = false;
        prev_regbuf = regbuf;

        memcpy(scratch, &bkpb, sizeof(XLogRecordBlockHeader));
        scratch += sizeof(XLogRecordBlockHeader);
        if (!samerel):
            memcpy(scratch, &regbuf->rnode, sizeof(RelFileNode));
            scratch += sizeof(RelFileNode);
        memcpy(scratch, &regbuf->block, sizeof(BlockNumber));
        scratch += sizeof(BlockNumber);
    }

    if (mainrdata_len > 0)
        if (mainrdata_len > 255)
            *(scratch++) = XLR_BLOCK_ID_DATA_LONG; /* 255 */
            memcpy(scratch, &mainrdata_len, sizeof(uint32));
            scratch += sizeof(uint32);
        else:
            *(scratch++) = XLR_BLOCK_ID_DATA_SHORT; /* 254 */
            *(scratch++) = (uint8) mainrdata_len;
        
        rdt_datas_last->next = mainrdata_head;
		rdt_datas_last = mainrdata_last;
		total_len += mainrdata_len;
    rdt_datas_last->next = NULL;

    hdr_rdt.len = (scratch - hdr_scratch);
    total_len += hdr_rdt.len;

    COMP_CRC32C(rdata_crc, hdr_scratch + sizeof(XLogRecord), hdr_rdt.len - sizeof(XLogRecord));
	for (rdt = hdr_rdt.next; rdt != NULL; rdt = rdt->next)
		COMP_CRC32C(rdata_crc, rdt->data, rdt->len);
    
	rechdr->xl_xid = GetCurrentTransactionIdIfAny();
	rechdr->xl_tot_len = total_len;
	rechdr->xl_info = info;
	rechdr->xl_rmid = rmid;
	rechdr->xl_prev = InvalidXLogRecPtr;
	rechdr->xl_crc = rdata_crc;

    return &hdr_rdt;

XLogRecPtr XLogInsertRecord(XLogRecData rdata, XLogRecPtr fpw_lsn)
    XLogCtlInsert *Insert = &XLogCtl->Insert;
    XLogRecord *rechdr = (XLogRecord *) rdata->data;

    isLogSwitch = rechdr->xl_rmid == RM_XLOG_ID && rechdr->xl_info == XLOG_SWITCH
    if isLogSwitch:
        WALInsertLockAcquireExclusive()
    else:
        WALInsertLockAcquire()
    
    if RedoRecPtr != Insert->RedoRecPtr:
        RedoRecPtr = Insert->RedoRecPtr;
    doPageWrites = Insert->fullPageWrites || Insert->forcePageWrites;

    XLogRecPtr	StartPos;
    XLogRecPtr	EndPos;
    if isLogSwitch:
        ReserveXLogSwitch(&StartPos, &EndPos, &rechdr->xl_prev)
    else:
        ReserveXLogInsertLocation(rechdr->xl_tot_len, &StartPos, &EndPos, &rechdr->xl_prev)

    WALInsertLockRelease();
    MarkCurrentTransactionIdLoggedIfAny();

    /* Update shared LogwrtRqst.Write, if we crossed page boundary */
    if StartPos / XLOG_BLCKSZ != EndPos / XLOG_BLCKSZ:
        if XLogCtl->LogwrtRqst.Write < EndPos:
            XLogCtl->LogwrtRqst.Write = EndPos;
        LogwrtResult = XLogCtl->LogwrtResult;
    
    if isLogSwitch:
        XLogFlush(EndPos)
        TODO
    
    ProcLastRecPtr = StartPos;
    XactLastRecEnd = EndPos;
```

# 2 xlog重放
```c
XLogReaderState *xlogreader = XLogReaderAllocate();
    XLogRecord record = ReadRecord(xlogreader);
    do {
        RmgrTable[record->xl_rmid].rm_redo(xlogreader);
        record = ReadRecord(xlogreader);
    }

heap_redo(XLogReaderState *record)
    uint8 info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;
    switch info & XLOG_HEAP_OPMASK:
        case XLOG_HEAP_INSERT:
            heap_xlog_insert(record)
        case XLOG_HEAP_DELETE:
            TODO

heap_xlog_insert(XLogReaderState *record)
    XLogRecPtr	lsn = record->EndRecPtr;

    RelFileNode target_node;
    XLogRecGetBlockTag(record, 0, &target_node, NULL, &blkno);

    /*  set tup's offset */
    ItemPointerData target_tid;
    ItemPointerSetBlockNumber(&target_tid, blkno);
    ItemPointerSetOffsetNumber(&target_tid, xlrec->offnum);

    xl_heap_insert *xlrec = (xl_heap_insert *) XLogRecGetData(record);
    if XLogRecGetInfo(record) = XLOG_HEAP_INIT_PAGE
        buffer = XLogInitBufferForRedo(record)
        page = BufferGetPage(buffer)
        PageInit(page)
    else
        action = XLogReadBufferForRedo(record, &buffer)
    
    
```