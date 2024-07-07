---
title: PostgreSQL 3-8 Index
date: 2022-09-25 16:38:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

[toc]

# 1 背景
# 1.1 数据检索
通常，应用会使用数据库存储大量数据，比如，单个表存储上百万、甚至上百亿条数据。此时，对表中数据进行某些查询时，将变得非常困难，比如等值查询、范围查询、模糊查询等。

# 1 索引概述
## 1.1 概述
- 索引方式：postgresql共有5种索引
    - 唯一索引：不能出现重复的值
    - 主键索引：主键自动创建唯一索引
    - 多属性索引：多个列（最多32列）
    - 部分索引：WHERE过滤索引
        ```sql
        -- 示例
        CREATE INDEX .. WHERE (c1 > 10);
        ```
    - 表达式索引：和部分索引没太大区别，都是额外调用1个函数
        ```sql
        -- 示例
        CREATE INDEX .. WHERE (c1 = 10);
        ```

- 索引分类：postgresql有多种索引，本文见介绍以下4种
    - btree
    - hash
    - gist
    - gin

- 索引系统表
    - pg_am：存储所有索引的方法
    ```sql
    SELECT * FROM pg_am;
        amname |  amhandler  | amtype
        -------+-------------+--------
        btree  | bthandler   | i
        hash   | hashhandler | i
        gist   | gisthandler | i
        gin    | ginhandler  | i
        spgist | spghandler  | i
        brin   | brinhandler | i
    ```
    - pg_index：存储所有索引，索引列的下标
    ```sql
    SELECT indexrelid,indrelid,indnatts,indkey FROM pg_index;
        indexrelid | indrelid | indnatts |  indkey
        -----------+----------+----------+----------
              2831 |     2830 |        2 | 1 2
    ```

## 1.2  索引函数
### 1.2.1 调用索引函数
```python
# 1 创建索引
DefineIndex()
    index_create() # 向系统表中写入tuple
    index_open()
    index_build()
        ambuild -> OidFunctionCall3()
    index_close()
    validate_index()

# 2 索引写入数据
ExecInsert()
    heap_insert()
    ExecInsertIndexTuples()
        index_insert()
            aminsert -> FunctionCall6()

# 3 索引扫描数据
ExecIndexScan()
    IndexNext()
        index_getnext()
            index_getnext_tid()
                amgettuple -> FunctionCall2()
            index_fetch_heap()

# 4 更新索引数据
ExecUpdate()
    heap_update()
    ExecInsertIndexTuples()
            index_insert()

# 5 删除索引
ExecDelete()
    heap_delete()
```

### 1.2.2 索引函数
pg_am系统表存储统一接口
| 类型 | btree | hash | gist | gin | spgist | brin |
|-|-|-|-|-|-|-|
aminsert    | btinsert      |
ambeginscan | btbeginscan   |
amgettuple  | btgettuple    |
amgetbitmap | btgetbitmap   |
amrescan    | btrescan      |
amendscan   | btendscan     |
ammarkpos   | btmarkpos     |
ambuild     | btbuild       |
ambuildempty| btbuildempty  |
ambulkdelete| btbulkdelete  |
amvacuumcleanup| btvacuumcleanup|
amcanreturn | btcanreturn   |
amcostestimate | btcostestimate |
amoptions   | btoptions     |

# 2 索引实现
## 2.1 创建索引 ambuild
创建索引的关键流程如下：
1. 扫描所有数据，排序
2. 自底向上构建B-Link树，Root节点不断变化
3. 每个节点存储High-Key，存储右兄弟
### 一、build整体流程
```python
btbuild
    _bt_spoolinit # 1 初始化spool
    _bt_spoolinit # 如果是唯一索引，再初始化一个spool2，存储被标记删除的tuple

    IndexBuildHeapScan # 2 扫描基表所有tuple
        IndexBuildHeapRangeScan
            heap_beginscan_strat
            heap_getnext
            btbuildCallback <- callback # 3 将基表tuple存spool
                _bt_spool
                    tuplesort_putindextuplevalues
                        index_form_tuple
                        puttuple_common
            heap_endscan

    _bt_leafbuild
        tuplesort_performsort # 3 排序所有tuple
        _bt_load
            # 如果是唯一索引，归并spool和spool2，再建索引
            tuplesort_getindextuple # 4 遍历获取spool中所有tuple
            _bt_buildadd # 5 构建b+树 （详细逻辑如下）
            _bt_uppershutdown
            smgrimmedsync
    _bt_spooldestroy
```

### 二、build插入tuple
b+树的每个节点都是1个page，为让build的流程更直观一点，用以下格式表示1个page
```bash
# 假设每个page能存储4个tuple

# 每个page的内容如下
Pageno                          BTPageOpaque (pre, next)
+---+----------------+---------+-----+
| - | -  -  -  -  -  | - - - - | - - |
+---+----------------+---------+-----+
    ItemPointer      ItemData
```

在每个page中，有以下约定：
- 第1个ItemPointer为highkey，存储本page的最大值，只有page满时，才会设置highkey
- 第2个ItemPointer为firstkey，存储本page的最小值
- 每个page的最大值，会存在3个地方：本page highkey，下一page firstkey，父母节点

_bt_buildadd 构建b+树的函数有如下2个场景：
1. 场景1 有空闲空间，直接插入Page
```python
# input: | 1 | - - - - - | - - - - | - - |
_bt_buildadd(1)
    if PageGetFreeSpace < itupsz:
        ...
    _bt_sortaddtup
        PageAddItem
# output: | 1 | - p1 - - - | - - - 1 | - - |
```

2. 场景2 无空闲空间，分裂
自底向上构建二叉树，root-page是会不断改变的
```python
# input: | 1 | - p1 p3 p5 p7 | 7 5 3 1 | - - |
_bt_buildadd(9)
    if PageGetFreeSpace < itupsz: # 无空闲空间
        # 生成new-page，预留high-key位置
        newpage = _bt_blnewpage                           # | 2 | -  -  -  -  -  | - - - - | - - | newpage
        # 将old-page的最大值，写入new-page的第一个位置
        _bt_sortaddtup(newpage, PageGetItem(last_off))    # | 2 | -  p7 -  -  -  | - - - 7 | - - | newpage
        # 把最大指移入high-key的位置
        *(oldpage, P_HIKEY) = *(oldpage, last_off)        # | 1 | p7 p1 p3 p5 p7 | 7 5 3 1 | - - | oldpage
        # 删除old-page中的重复指针
        oldpage->pd_lower -= sizeof(ItemIdData)           # | 1 | p7 p1 p3 p5 |    7 5 3 1 | - - | oldpage

        # 从栈中，找到parent-page，如果无parent-page，则创建parent-page 
        if state->btps_next is NULL:
            state->btps_next = _bt_pagestate
                _bt_blnewpage                             # | 3 | -  -  -  -  -  | - - - - | - - | parent-page
        # 调用_bt_buildadd，将oldpage中的最大值，插入到parent-page
        _bt_buildadd(btps_next, (oldpage, P_HIKEY))       # | 3 | - p7  -  -  -  | - - - 7 | - - | parent-page
        # 注意：此处parent-page中，item-data存储的是子page的blockno和high-key，即此处的7 = {(1, 1), 7}

        oldpage->next = newpage                           # | 1 | p7 p1 p3 p5    | 7 5 3 1 | - 2 | oldpage
        newpage->prev = oldpage                           # | 2 | -  p7 -  -  -  | - - - 7 | 1 - | newpage

        # old-page 不会再修改，将old-page写入磁盘
        _bt_blwritepage 

        # 记录写入位置为新page的first
        last_off = P_FIRSTKEY

    # 将tuple写入new-page
    _bt_sortaddtup
        PageAddItem                                       # | 2 | -  p7 p9 -  -  | - - 9 7 | 1 - | newpage
    # 更新栈信息
    state->btps_page = npage;
    state->btps_lastoff = last_off
    state->btps_minkey: now is min of npage
# output:
#               | 3 | - p7  -  -  -  | - - - 7 | - - |
#                                           /
#                                          /
#                                   | 1 | p7 p1 p3 p5 | 7 5 3 1 | - 2 |    ---->      | 2 | -  p7 p9 -  -  | - - 9 7 | 1 - |         
```

## 2.2 索引写入数据 aminsert
```python
btinsert
    index_form_tuple
    _bt_doinsert
        _bt_mkscankey

        _bt_search    # 1 遍历B+树，查找tuple应插入的叶子page
            _bt_getroot # 获取root-page
            for:
                _bt_moveright # 并发场景中，判断Page是否分裂
                BTPageOpaque(page)
                if P_ISLEAF(BTPageOpaque): # 如果是叶子page，返回
                    break
                _bt_binsrch # 在中间page中，获取tuple
                blkno = PageGetItem->itup->t_tid # 从tuple中，获取子节点的blkno
                _bt_relandgetbuf(blkno) # 读取子节点
        _bt_moveright # 2 并发场景中，如果page发生分裂，获取右边的page

        _bt_binsrch   # 3 在page中，通过二分查找法，找到插入点
        _bt_check_unique
        if TransactionIdIsValid:
            _bt_relbuf    #  如果拿锁失败，从第1步开始重新遍历
            _bt_freestack
        _bt_findinsertloc # 4 获取插入位置
        _bt_insertonpg    # 5 插入tuple

        _bt_freestack
        _bt_freeskey
```

```python
# input:
#               | 3 | - p7  -  -  -  | - - - 7 | - - |
#                                           /
#                                          /
#                                   | 1 | p7 p1 p3 p5 | 7 5 3 1 | - 2 |    ---->      | 2 | -  p7 p9 -  -  | - - 9 7 | 1 - |

_bt_findinsertloc(6) # 已选中page为 page[1]
    lpage = page[1]
    while PageGetFreeSpace(page) < itemsz:
        #  如果当前page无空间，尽量回收当前page的空间
        if P_HAS_GARBAGE(lpage):
            _bt_vacuum_one_page
        # 如果tuple的值等于highkey，可以考虑将highkey写入右边的page，否则中止循环，后面的函数进行分裂
        if _bt_compare(P_HIKEY) != 0:
            break
        # 向右查找page，仅限于特殊场景
        rblkno = lpage->next
        rbuf = _bt_relandgetbuf(rblkno)
    # 使用二分查找，获取插入位置
    _bt_binsrch                     # | 1 | p7 p1 p3 p5   | 7 5 3 1 | - 2 |
                                    #                         ^
```

- 场景一：需要分裂
```python
_bt_insertonpg(6)
    lpage = page[1]
    if PageGetFreeSpace(page) < itemsz:
        # 在lpage中查找分裂的位置
        _bt_findsplitloc
            # 判断分裂后新tuple在left还是right page   # | 1 | p7 p1 p3 p5   | 7 5 3 1 | - 2 |
            for offnum in (firstkey, maxoff):
                if offnum > newitemoff:
                    _bt_checksplitloc
                elif offnum < newitemoff:
                    _bt_checksplitloc
                else:
                    _bt_checksplitloc
                    _bt_checksplitloc
        # 分裂
        rbuf = _bt_split
            # 生成新的righe page        
            rbuf = _bt_getbuf(P_NEW)                    # | 4 | -  -  -  -  -  | - - - - | - - | righepage (new)

            # 将原始的leftpage中的数据，分到新的page中
            origpage = lpage = page[1]                  # | 1 | p7 p1 p3 p5    | 7 5 3 1 | - 2 |
            leftpage = PageGetTempPage(origpage)        # | - | -  -  -  -  -  | - - - - | - - |
            rightpage = rbuf                            # | 4 | -  -  -  -  -  | - - - - | - - |
            _bt_pageinit(leftpage)
            for i in (FIRST_KEY(origpage), maxoff):
                if i < firstright:                      # | - | p7 p1 p3 p5 -  | - 5 3 1 | - 2 |
                    _bt_pgaddtup(leftpage)
                elif i == newitemoff:                   # | 4 | -  p6 p6 -  -  | - - 7 6 | - - |
                    if newitemonleft:
                        _bt_pgaddtup(leftpage)
                    else:
                        _bt_pgaddtup(rightpage)
                else:
                    _bt_pgaddtup(rightpage)
            
            # 更新原始的right的prev指针为新的right
            sbuf = _bt_getbuf(origpage->next)
            sbuf->prev = rightpage                      # | 2 | -  p7 p9 -  -  | - - 9 7 | 4 - |

            # 将临时的leftpage，覆盖原始的leftpage
            PageRestoreTempPage                         # | 1 | p7 p1 p3 p5 -  | - 5 3 1 | - - |

            # 记录分裂的xlog
            XLogRegisterBuffer(buf)
            XLogRegisterBuffer(rbuf)
            XLogRegisterBuffer(sbuf)
            XLogInsert(RM_BTREE_ID)

        # 将lbuf和rbuf的值，更新到父母节点
        _bt_insert_parent
            # 从栈中，获取parrent-page
            pbuf = _bt_getstackbuf
            # todo：如何为新的page设置highkey
            # 将新的highkey插入parrent-page
            _bt_insertonpg()
# output: todo
```

- 场景二：无需分裂
```bash
_bt_insertonpg(6)
    _bt_pgaddtup
     XLogInsert(RM_BTREE_ID)
```