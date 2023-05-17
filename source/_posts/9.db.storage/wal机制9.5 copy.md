---
title: 存储引擎 9.5 WAL机制
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

# 1 xlog格式
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
} XlogRecData;

/* 进程级全局变量：

+-------------+-------------+-------------+
| XLogRecData | XLogRecData | ...         |
+-------------+-------------+-------------+
              ^ last
*/

XLogRecData *rdatas; /* XLogRecData 数组 */
int num_rdatas;
int max_rdatas;

XLogRecData *mainrdata_head;
XLogRecData *mainrdata_last;
uint32 mainrdata_len; /* chain中的数据总长度 */

XLogRegisterData(data, len)
    XLogRecData *rdata = rdatas[num_rdatas++];
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

+-------------+-------------+-------------+
| XLogRecData | XLogRecData | ...         |
+-------------+-------------+-------------+

*/

registered_buffer *registered_buffers;
int max_registered_buffers;
int max_registered_block_id = 0;

XLogRegisterBuffer(block_id, buffer, flags)
    
```