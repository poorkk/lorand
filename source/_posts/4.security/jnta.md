1. sk中包含等号，使用环境变量解析key=value时，需忽略等号。
   ```c
   if (curchr == '=' && !issk) {
    ...
    if (strlen(km_str_strip(scan->key)) > 0 && strcasecmp(km_str_strip(scan->key), "sk") == 0) {
        issk = true;
    }
   }
   ```
2. huawei_kms检查算法时，支持sm4
3. huawei_kms的http请求头中不能设置porjectId，不设置conten_length
4. 构造http请求头和解析http响应体时，不使用sha_encode，使用hex_encode
5. 构造http请求头时，decrypt-datakey时，plainlen的长度设置为32，而非“cipher.len - PLAIN_LEN”
6. 密钥状态 = 1时，是active
7. 开启ENABLE_HUAWEI_KMS的宏