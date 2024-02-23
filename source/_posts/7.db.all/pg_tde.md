---
title: 数据库 5 postgresql 表级透明加密
date: 2022-09-25 16:03:37
categories:
    - 数据库
tags:
    - postgresql
---

# 1 特性背景

# 2 实现方案
## 2.1 前置条件
### 一、生成系统表
- pg_key_info 密钥信息

| oid | name | type | storage | access | protect | create_time | round |
| - | - | - | - | - | - | - | - |
| 1 | mk_1 | master_key | localkms | path=./keyfile | $pass1 | 2024/02/01 13:01:01 | - |
| 2 | dk_1 | date_key | database | 16000:1:8190 | mk1 | 2024/02/01 13:01:01 | 0 |

- pg_key_data 密钥

| oid | keydata | algorithm |
| - | - | - |
| 3 | abcd1234 | aes_256 |

### 二、生成系统视图
- pg_enc_rels

| oid | database | schema | relname | data_key | master_key | algorithm |
| - | - | - | - | - | - | - |
| 8 | postgres | public | t1 | dk_1 | mk_1 | aes_256 |

## 2.2 操作步骤
### 一、定义密钥
1. 定义主密钥
```sql
CREATE KEY mk_1 (type=master_key, storage=localkms, access='./keyfile', protect='$pass1');
```
2. 定义数据密钥
```sql
CREATE KEY dk_1 (type=date_key, procect=mk_1, algothm=aes_256);
```

### 二、定义加密表
1. 定义加密表
```sql
CREATE TABLE t1 (c1 INT, c2 TEXT) with (encrypt=dk_1);
```

2. 定义索引
```sql
CREATE INDEX i1 ON t1 (c1);
```

### 三、操作加密表
1. 写入数据
```sql
INSERT INTO t1 VALUES (1, 'aaaaa');
```
2. 查询数据
```sql
SELECT * FROM t1;
```
3. 轮转密钥
```sql
ALTER KEY dk_1 UPDATE (keydata);
```

# 3 关键设计
## 3.1 格式设计
- Page格式
```c
+---------------+
| PageHeader    | 在Header中，标记Page属于加密表，并标记加密状态
|               |
| tuple ..      |
| tuple 2       |
| tuple 1       |
| PageSpecial   | 在Special中，记录密钥信息
+---------------+
```
- PageSpecial格式
```c
typedef struct {
    uint8 version;
    Tid   datakeypos;
    uchar iv[16];
    uchar mac[32];
} PageEncInfo;
```

## 3.2 流程设计
### 整体流程
```java
@user
    create table t1 (.. dk_1)
@user:
    insert into t1 (..abc)
@kernel.postgres:
    heap_insert(t1, 'abc')
    page_init(t1.dk_1, page)
        memcpy(page, get_enc_info(t1.dk_1))
        page_set_flag(page, tde_flag)
    page_add_item(page, 'abc')
@kernel.checkpoint:
    smgrwrite(page)
        if (is_page_set_flag(page, tde_flag))
            page_encrypt_date(page)
```
### 加密粒度
### 加密时机
### 加密判断
### 密钥存储
### 解密时机
### 解密判断
### 密钥来源

## 3.3 其他设计

## 3.2 主备密钥同步
```python
# 主机
page_init(page, encinfo)
    memcpy(page, encinfo)
```
