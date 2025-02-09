---
title: 工具 配置新环境
date: 2022-09-25 16:03:37
categories:
    - 工具
tags:
    - 博客
---

1. 安装code-serve
    - https://blog.zhaokeyong.cn/archives/CentOScode-server
    安装3.4版本，方便
    ```bash
    # 1 从本地机器上，找到code-server所在文件夹
    cd /home/shenkun/vscode
    mkdir allpkg
    # 放插件的目录
    cp -r ~/.local allpkg
    cp -r ./code-server allpkg
    tar -zcvf allpkg.tar.gz allpkg

    cd /home/shenkun/vscode
    tar -zxvf allpkg.tar.gz
    mv allpkg/code-server ./
    rm -rf ~/.local/share/code-server/*
    mv allpkg/.local/share/code-server/* ~/.local/share/code-server

    nohup ./code-server &

    # 设置配置文件
    vi ~/.config/code-server/config.yaml
    bind-addr: 0.0.0.0:3000
    auth: password
    password: password
    cert: false

    nohup ./code-server &
    ```
    常用插件：
    - c/c++
    - gitlens
    - markdown preview enhanced
2. 安装 oracle
    - https://www.tapd.cn/60475194/markdown_wikis/show/#1160475194001006330