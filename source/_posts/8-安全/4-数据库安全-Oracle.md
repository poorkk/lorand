---
title: 数据库安全 Oracle
categories:
    - 安全
tags:
    - 安全
---

# 7 加密函数
DBMS_OBFUSCATION_TOOLKIT 

dbms_obfuscation_toolkit
- 函数格式
```sql
dbms_obfuscation_toolkit.DESencrypt(plain, key, iv);
```
- 函数使用
```sql
```

```sql
CREATE TABLE t1 (c1 INT, c2 VARCHAR2(10));
INSERT INTO t1 VALUES (1, upper('abcc'));
SELECT c1,c2,lower(c2) FROM t1;
```

问题：
```sql
-- 3des
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401050110974949, which=>0)) from dual;
-- 密钥长度为12字节，实际只用了10字节，后面的被截断扔了
-- 明文为8字节，密文为8字节
-- 输出：
E7E14DFD9E0DED3B

-- 明文7，9字节都不行，只能是8的整数倍
-- 明文16字节
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('1111111111111111', key_string=>010199010401050110974949, which=>0)) from dual;
-- 输出：
E7E14DFD9E0DED3B D80084E1EDD76DF1

-- 密钥截断
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401050110, which=>0)) from dual;
-- 输出：
E7E14DFD9E0DED3B

-- 最低为18位，后几位没用，实际为14位
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401050000, which=>0)) from dual;
-- 输出：
E7E14DFD9E0DED3B

-- 18字节密钥，截断6位，明文变长
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('1111111111111111', key_string=>010199010401050000, which=>0)) from dual;
-- 输出：
E7E14DFD9E0DED3B D80084E1EDD76DF1

-- 14最后一位变化也不影响
select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401040000, which=>0)) from dual;
-- 输出：
E7E14DFD9E0DED3B

select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401030000, which=>0)) from dual;
-- 输出
C725C0228F108C77
-- 20也是
C725C0228F108C77

select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401010000, which=>0)) from dual;
-- 输出
69AB538F1542F919
```