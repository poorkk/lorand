---
title: db.pg.源码安装postgreSQL mac
date: 2022-09-25 16:03:37
categories:
    - 数据库
    - postgresql
tags:
    - postgresql
---

# 源码安装postgresql

**摘要**
> 本文介绍，在MacOS操作系统中，如何基于VwWare虚拟机，搭建Linux环境，并在Linux中通过源码编译postgreSQL

**目录**
[toc]

# 1 背景
## 1.1 问题
在macOS中，下载并配置postgreSQL依赖的开源组件时，频繁碰壁.遂决定租一台云服务主机，编译并运行postgreSQL，以达到学习的目的

## 1.2 云服务
| 云服务商 | 0元试用 | 条件 | 最低配置年费 |
|-|-|-|-|
| 阿里云 | 有 | 仅新用户有一次机会 | 1core 2g 24/3mon |
| 华为云 | 有 | 每日9.30开抢 |  |
| 腾讯云 | 有 | 仅新用户有一次机会 | 2core 2g 6.6/mon |
| 天翼云 | 无 | 低价秒杀 1core 2g 33/year | 1core 2g 122/year |
| 金山云 | 无 | 无 | 2core 4g 576/year |
| 移动云 | 有 | 仅新用户有一次机会 | 2core 4g 28/mon |
| 百度云 | 无 | 无 | 1core 1g 34/mon |
| 浪潮云 | 无 | 无 | 垃圾 |

## 1.3 阿里云
使用centOS 7.6主机
需先更改root密码
使用命令行
ssh root@ip

yum install git

创建新用户
useradd lorand
passwd lorand
gauss@123

ssh lorand@{IP}

安装code-server
-- fail
https://cloud.tencent.com/developer/article/1655175

-- fail
git clone https://gitee.com/mirrors/code-server.git
https://gitee.com/mirrors/code-server?_from=gitee_search
curl -fsSL https://code-server.dev/install.sh | sh


-- succeed https://zhuanlan.zhihu.com/p/493434939
https://link.zhihu.com/?target=https%3A//github.com/coder/code-server/releases/download/v4.2.0/code-server_4.2.0_amd64.deb

scp ./code-server_4.2.0_amd64.deb lorand@182.92.66.91:~

su
yum install dpkg
dpkg -i code-server_4.2.0_amd64.deb

exit
vi ~/.config/code-server/config.yaml
bind-addr: 0.0.0.0:3000

code-server 或 nohup code-server &

浏览器：182.92.66.91：3000

在”网络与安全“->"安全组"中，新增防火墙规则，目的端口：3000，允许连接

182.92.66.91：3000


## 2 下载postgreSQL源码
su
yum install -y readline readline-devel openssl openssl-devel zlib zlib-devel bison bison-devel flex flex-devel

exit
git clone https://gitee.com/lorand/postgresql_9.2.4.git

echo 'export BUILD_ROOT=`pwd`' > pgenv
echo 'export PG_HOME=$BUILD_ROOT/install' >> pgenv
echo 'export PATH="$PG_HOME/bin:$PATH"' >> pgenv

```bash
source pgenv
cd postgresql-9.2.4
./configure --prefix=$PG_HOME --enable-debug
make install -sj
mkdir $PG_HOME/log && touch $PG_HOME/log/server.log
initdb -D $PG_HOME/data
pg_ctl -D $PG_HOME/data/ -l $PG_HOME/log/server.log start
ps ux | grep bin/postgres

psql -d postgres
CREATE USER pglorand WITH PASSWORD 'gauss@123';

新增安全组：目的端口：5432

vi pg_hba.conf
host all all 0.0.0.0/0 trust
vi postgres.conf
listen_addresses = ’*‘

psql -d postgres -U pglorand
```

{% asset_img install.succeed.png %}