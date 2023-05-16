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
heap_insert场景
```c
XLogBeginInsert() /* 检查受否在故障恢复 */
XLogRegisterData(data, datasz) /*  */
XLogRecPtr XLogInsert(RmgrId rmid, uint8 info)
XLogFlush(XLogRecPtr record)
```