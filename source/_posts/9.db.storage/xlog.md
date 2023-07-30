---
title: 存储引擎 xlog
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

```c
// 1 插入阶段
heap_insert()
    LogBeginInsert()
    XLogRegisterData(data, datasz)
    XLogRegisterBuffer(0, buffer, bufflags)
    XLogRegisterBufData0(0, data, datasz)
    XLogRegisterBufData(o, data, datasz)
    XLogIncludeOrigin()
    XLogRecPtr = XLogInsert(RM_HEAP_ID, XLOG_HEAP_INSERT | XLOG_HEAP_INIT_PAGE)

// 2 组装阶段 - 调用assemble
XLogInsert()
    XLogRecPtr RedoRecPtr
    XLogRecPtr fpw_lsn
    GetFullPageWriteInfo(&RedoRecPtr)
    XLogRecData *tdt = XLogRecordAssemble(rmid, info, doPageWrites, &fpw_lsn)
    XLogRecPtr EndPos = XLogInsertRecord(rdt, fpw_lsn)

// assemble 返回值
typedef struct XLogRecData {
    struct XLogRecData *next;
    char *data;
    uint32 len;
} XLogRecData;  /* 物理xlog：header + main的list */

typedef uint64 XLogRecPtr;

// 物理xlog的头部
typedef struct XLogRecord {
    uint32 xl_tot_len;
    TransactionId xl_xid;
    XLogRecPtr xl_prev; /* 前一条物理xlog */
    uint8 xl_info;
    RmgrId xl_rmid;
    pg_crc32c xl_crc;
} XLogRecord;

// 2.1 assemble 组装
XLogRecData *XLogRecordAssemble(rmid, info, doPageWrites) /* RM_HEAP_ID, XLOG_HEAP_INSERT | XLOG_HEAP_INIT_PAGE */
    for (block_id = 0; block_id < max_registered_block_id; block_id++) {
        // 1 依次获取 block / buffer
        registered_buffer *regbuf = &registered_buffers[block_id]

        // 2 判断是否写 full-page
        if :
            ...
        else:
            XLogRecPtr page_lsn = PageGetLSN(regbuf->page)
                needs_backup = (page_lsn <= RedoRecPtr) // page的lsn小于xlog的lsn
        
        // 3 构造 BlockHeader
        XLogRecordBlockHeader bkpb
        bkpb.id = block_id
        bkpb.fork_flags = regbuf->forkno
        if regbuf->flags & REGBUF_WILL_INIT:
            bkpb.fork_flags |= BKPBLOCK_WILL_INIT
        
        // 4 拼接 page image
        if needs_backup: // header后紧跟page imge
            Page page = regbuf->page
            if regbuf->flags & REGBUF_STANDARD: // 计算孔洞
                bimg.hole_offset = lower 
                cbimg.hole_length = upper - lower
            if wal_compression: // 压缩page
                is_compressed = XLogCompressBackupBlock(page, bimg.hole_offset, cbimg.hole_length, regbuf->compressed_page, &compressed_len)
            bkpb.fork_flags |= BKPBLOCK_HAS_IMAGE
            if is_compressed:
                // 物理数据 | XLogRecord--BlockNumber | regbuf->compressed_page
            else:
                bimg.length = BLCKSZ - cbimg.hole_length
                if cbimg.hole_length == 0
                    // 物理数据 | XLogRecord--BlockNumber | regbuf->page
                    rdt_datas_last->data = page
                    rdt_datas_last->len = BLCKSZ
                else:
                    // page分成两部分存：page起始到hole起始，hole结束到page结束
                    // 物理数据 | XLogRecord--BlockNumber | regbuf->page--hole_offset |   regbuf->page--hole_offset--hole_length |

        // 5 拼接 tuple data
        if needs_data:
            // // 物理数据 | XLogRecord--BlockNumber | regbuf->page | data1 | data2 | ... |
            bkpb.fork_flags |= BKPBLOCK_HAS_DATA
            bkpb.data_length = regbuf->rdata_len
            
            rdt_datas_last->next = regbuf->rdata_head
            rdt_datas_last = regbuf->rdata_tail

        // 6 继续填充 BlockHeader
        if prev_regbuf && RelFileNodeEquals(regbuf->rnode, prev_regbuf->rnode):
            samerel = true
            bkpb.fork_flags |= BKPBLOCK_SAME_REL
        prev_regbuf = regbuf
        // 物理数据：| XLogRecord  XLogRecordBlockHeader | regbuf->page | ...
        memcpy(scratch, &bkpb, sizeof(XLogRecordBlockHeader))
        
        // 7 继续填充 
        if needs_backup:
            // 物理数据：| XLogRecord XLogRecordBlockHeader XLogRecordBlockImageHeader | regbuf->page | ...
            memcpy(scratch, &bimg, sizeof(XLogRecordBlockImageHeader))
            if cbimg.hole_length != 0 && is_compressed:
                // 物理数据：| XLogRecord XLogRecordBlockHeader XLogRecordBlockImageHeader XLogRecordBlockCompressHeader | regbuf->page | ...
                memcpy(scratch, &cbimg, sizeof(XLogRecordBlockCompressHeader))
        if samerel:
            // 物理数据：| XLogRecord XLogRecordBlockHeader XLogRecordBlockImageHeader XLogRecordBlockCompressHeader RelFileNode | regbuf->page | ...
            memcpy(scratch, &regbuf->rnode, sizeof(RelFileNode))
        /*
         * 物理数据：（list的第一个node）
         *  | XLogRecord 
         *  | XLogRecordBlockHeader  -----------+ 可能重复多次
         *  | XLogRecordBlockImageHeader        |
         *  | XLogRecordBlockCompressHeader     |
         *  | RelFileNode                       |
         *  | BlockNumber ----------------------+
         *  | ... 重复
         */ 
        memcpy(scratch, &regbuf->block, sizeof(BlockNumber))
    } // 结束遍历 Block

    /*
    * 物理数据：（list的第一个node）
    *  | XLogRecord 
    *  | XLogRecordBlockHeader  -----------+ 可能重复多次
    *  | XLogRecordBlockImageHeader        |
    *  | XLogRecordBlockCompressHeader     |
    *  | RelFileNode                       |
    *  | BlockNumber ----------------------+
    *  | ... 重复
    *  | XLR_BLOCK_ID_ORIGIN
    *  | replorigin_session_origin (uint16)
    *  | XLR_BLOCK_ID_DATA_SHORT
    *  | mainrdata_len
    */ 
    if include_origin && replorigin_session_origin:
        *(scratch++) = XLR_BLOCK_ID_ORIGIN
        memcpy(scratch, &replorigin_session_origin, sizeof(replorigin_session_origin))
    
    // 4 拼接 maindata
        /*
        * [XLogRecord-...-BlockNumber] -> [page1] -> [page2]-> [page1 data1] -> [page2 data2] ->  [maindata] -> [maindata] -> [...]
        */
        if mainrdata_len > 0 :
            *(scratch++) = XLR_BLOCK_ID_DATA_LONG / XLR_BLOCK_ID_DATA_SHORT
            memcpy(scratch, &mainrdata_len, sizeof(uint32))        

            rdt_datas_last->next = mainrdata_head
            rdt_datas_last = mainrdata_last
```

xlog完整格式
```bash
# 物理xlog
|   list 节点                   | 是否必选                 描述
+-------------------------------+---+-----------------------------------------------------------------
| XLogRecord                    | Y |
|   << loop start >>            | - |
| XLogRecordBlockHeader         | N | 有page时 {id, fork_flags, data_length} : INIT_PAGE HAS_IMAGE HAS_DATA
| XLogRecordBlockImageHeader    | N | 有page时，写入full-page镜像
| XLogRecordBlockCompressHeader | N | 有page时，写入full-page镜像，开启压缩
| RelFileNode                   | N | 有page时
| BlockNumber                   | N | 有page时
|   << loop end >>              | - |
| XLR_BLOCK_ID_ORIGIN           | N |
| replorigin_session_origin     | N |
| XLR_BLOCK_ID_DATA_SHORT/LONG  | N | 有maindata时（可能有误）
| mainrdata_len                 | N | 有maindata时
+-------------------------------|---|-----------------------------------------------------------------
| page start -- hole start      | N | 有page时，写入full-page镜像
+-------------------------------|---|-----------------------------------------------------------------
| hole end -- page end          | N |
+-------------------------------|---|-----------------------------------------------------------------
| block1_data1                  | N | xl_heap_header {t_infomask2, t_infomask, t_hoff} : 记录tuple信息
+-------------------------------|---|-----------------------------------------------------------------
| block1_data2                  | N | tuple->t_data : 记录tuple数据
+-------------------------------|---|-----------------------------------------------------------------
| block2_data1                  | N |
+-------------------------------|---|-----------------------------------------------------------------
| block...                      | N |
+-------------------------------|---|-----------------------------------------------------------------
| maindata1                     | N | xl_heap_insert {OffsetNumber, flags} ： 记录tuple位置
+-------------------------------|---|-----------------------------------------------------------------
| maindata2                     | N |
+-------------------------------|---|-----------------------------------------------------------------
| maindata...                   | N |
+-------------------------------|---|-----------------------------------------------------------------
```