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
> 本文介绍，在Windows操作系统中，如何基于VwWare虚拟机，搭建Linux环境，并在Linux中通过源码编译postgreSQL

**目录**
[toc]

## 1 搭建环境
在本部分，介绍如何准备centOS虚拟机，并安装git、VSCode、gdb
1. 已有windows 10 x86物理机
2. window 10中，安装vmware
3. mware中，安装centos虚拟机
   > http://mirrors.huaweicloud.com/centos/8.4.2105/isos/x86_64/ 
4. centOS中，安装git，以及中文输入法
5. centOS中，安装VSCode
   > https://code.visualstudio.com/docs/setup/linux#_rhel-fedora-and-centos-based-distributions
6. centOS中，安装PostgresSQL依赖的第三方开源软件
   > sudo yum install -y readline readline-devel openssl openssl-devel zlib zlib-devel
   > # 部分linux可能会裁剪应用，因此可能需要安装如下软件
   > sudo yum install -y bison bison-devel flex flex-devel
   > [参考](https://blog.csdn.net/Linjingke32/article/details/80393576)
7. 安装调试工具
   > sudo yum install gdb

## 2 编译源码
在本部分，介绍如何在centOS中，下载、配置、编译、启动postgreSQL
1. 准备源码目录，配置环境变量
   > mkdir ~/pg_9.2.4
   > cd ~/pg_9.2.4
2. 下载PostgresSQL 9.2.4源码，并解压
   > wget https://ftp.postgresql.org/pub/source/v9.2.4/postgresql-9.2.4.tar.gz
   > (备用方法：https://www.postgresql.org/ftp/source/v9.2.4/)
   > tar -zxvf postgresql-9.2.4.tar.gz
   > 备注：建议初始化为git仓库，并上传到云端备份，以防因误改代码至代码无法编译
3. 准备环境变量
   > echo 'export BUILD_ROOT=`pwd`' > pgenv
   > echo 'export PG_HOME=$BUILD_ROOT/install' >> pgenv
   > echo 'export PATH="$PG_HOME/bin:$PATH"' >> pgenv
   > source pgenv
4. 编译源码
   > cd postgresql-9.2.4
   > ./configure --prefix=$PG_HOME --enable-debug
   > make install -sj
5. 生成日志文件并启动数据库服务器
   > mkdir $PG_HOME/log && touch $PG_HOME/log/server.log
   > initdb -D $PG_HOME/data
   > pg_ctl -D $PG_HOME/data/ -l $PG_HOME/log/server.log start
   > ps ux | grep bin/postgres
6. 连接数据库
   > psql -h localhost -p 5432 -d postgres
