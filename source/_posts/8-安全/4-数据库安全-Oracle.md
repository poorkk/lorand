---
title: 数据库安全 Oracle
categories:
    - 安全
tags:
    - 安全
---

https://www.tapd.cn/60475194/markdown_wikis/show/#1160475194001006330

# 7 加密函数
## 7.1 DES
- 函数格式
    ```sql
    dbms_obfuscation_toolkit.desencrypt(plain, key, cipher);
    ```
- 前置知识
    ```sql
    -- varchar --> raw
    SELECT utl_raw.cast_to_raw('11111111') FROM dual;
    -- raw --> varchar
    SELECT utl_raw.cast_to_varchar2(hextoraw('3131313131313131')) FROM dual;
    -- raw -> hex
    SELECT rawtohex('11111111') FROM dual;
    SELECT hextoraw('3131313131313131') FROM dual;
    ```
- 加密函数
    - 示例1
    ```sql
    SET SERVEROUTPUT ON;

    DECLARE
        cipher RAW(100);
    BEGIN
        dbms_obfuscation_toolkit.desencrypt(hextoraw('3131313131313131'), hextoraw('3131313131313131'), encrypted_data=>cipher);
        dbms_output.put_line(rawtohex(cipher));
    END;
    /
    ```
    - 示例2
    ```sql
    CREATE OR REPLACE FUNCTION desencrypt(hex_plain VARCHAR2, hex_key VARCHAR2)
    RETURN VARCHAR2 IS
        cipher RAW(100);
    BEGIN
        dbms_obfuscation_toolkit.desencrypt(hextoraw(hex_plain), hextoraw(hex_key), encrypted_data=>cipher);
        RETURN rawtohex(cipher);
    END;
    /

    SELECT desencrypt('3131313131313131', '3131313131313131') FROM dual;
    -- 输出 655EA628CF62585F
    SELECT desencrypt('31313131313131313131313131313131', '3131313131313131') FROM dual;
    -- 输出 655EA628CF62585F20D3B8A6E657876B
    ```
- 解密函数
    - 示例2
    ```sql
    CREATE OR REPLACE FUNCTION desdecrypt(hex_cipher VARCHAR2, hex_key VARCHAR2)
    RETURN VARCHAR2 IS
        plain RAW(100);
    BEGIN
        dbms_obfuscation_toolkit.desdecrypt(hextoraw(hex_cipher), hextoraw(hex_key), decrypted_data=>plain);
        RETURN rawtohex(plain);
    END;
    /

    SELECT desdecrypt('655EA628CF62585F', '3131313131313131') FROM dual;
    -- 输出 3131313131313131
    SELECT desdecrypt('655EA628CF62585F20D3B8A6E657876B', '3131313131313131') FROM dual;
    -- 输出 31313131313131313131313131313131
    ```

## 7.2 3DES
- 函数格式
    ```sql
    dbms_obfuscation_toolkit.DESencrypt(plain, key, mode);
    ```
- 加密函数
    - 示例1
    ```sql
    select utl_raw.cast_to_raw(dbms_obfuscation_toolkit.des3encrypt('11111111', key_string=>010199010401050110974949, which=>0)) from dual;
    -- 输出：
    E7E14DFD9E0DED3B
    ```
- 其他
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