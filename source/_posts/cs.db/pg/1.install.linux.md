---
title: db.pg.源码安装postgreSQL
date: 2022-09-25 16:03:37
categories:
    - 数据库
    - postgresql
tags:
    - postgresql
---

# 源码安装postgresql

**摘要**
> 本文介绍，在Windows操作系统中，如何基于VwWare虚拟机，搭建Linux环境。并在Linux中，通过postgreSQL源码进行编译安装。

**目录**
[toc]

## 1 搭建环境
### 1.1 目的
通过postgreSQL源码进行编译安装，主要目的是：
1. 方便通过源码调试跟踪
2. 直接修改以增加调试信息或新功能  

### 1.2 准备
首先，介绍如何准备centOS虚拟机，并安装git、VSCode、gdb等工具
1. 已有windows 10/11 x86物理机
2. window 10中，安装vmware
3. vmware中，安装centOS虚拟机
   > http://mirrors.huaweicloud.com/centos/8.4.2105/isos/x86_64/ 
4. centOS中，安装git，以及中文输入法
5. centOS中，安装VSCode
   > https://code.visualstudio.com/docs/setup/linux#_rhel-fedora-and-centos-based-distributions
6. centOS中，安装PostgresSQL依赖的第三方开源软件
```shell
sudo yum install -y readline readline-devel openssl openssl-devel zlib zlib-devel

# 但是，部分linux可能会裁剪应用，因此可能需要安装如下软件
sudo yum install -y bison bison-devel flex flex-devel

#[参考](https://blog.csdn.net/Linjingke32/article/details/80393576)
```
7. 安装调试工具
```shell
sudo yum install gdb
```
  
## 2 编译源码
接下俩，介绍如何在centOS中，下载、配置、编译、启动postgreSQL
1. 准备源码目录，配置环境变量
```shell
mkdir ~/pg_9.2.4
cd ~/pg_9.2.4
```
2. 下载PostgresSQL 9.2.4源码，并解压
```shell
wget https://ftp.postgresql.org/pub/source/v9.2.4/postgresql-9.2.4.tar.gz

# 备用方法：https://www.postgresql.org/ftp/source/v9.2.4/
tar -zxvf postgresql-9.2.4.tar.gz

# 备注：建议初始化为git仓库，并上传到云端备份，以防因误改代码至代码无法编译
```
3. 准备环境变量
```shell
echo 'export BUILD_ROOT=`pwd`' > pgenv
echo 'export PG_HOME=$BUILD_ROOT/install' >> pgenv
echo 'export PATH="$PG_HOME/bin:$PATH"' >> pgenv
source pgenv
```
4. 编译源码
```shell
cd postgresql-9.2.4
./configure --prefix=$PG_HOME --enable-debug
make install -sj
```
1. 生成日志文件并启动数据库服务器
```shell
mkdir $PG_HOME/log && touch $PG_HOME/log/server.log
initdb -D $PG_HOME/data
pg_ctl -D $PG_HOME/data/ -l $PG_HOME/log/server.log start
ps ux | grep bin/postgres
```
2. 连接数据库
```shell
psql -h localhost -p 5432 -d postgres
```
