---
title: 计算机网络 socket编程
date: 2023-01-01 16:03:37
categories:
    - 计算机网络
tags:
    - 计算机网络
---

# 1 socket编程
## 1.1 基本接口
https://blog.csdn.net/hguisu/article/details/7445768/
```c
// server
int socket(int protofamily, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int close(int fd);

// 阻塞
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

// client
int socket(int protofamily, int type, int protocol);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// 结构体
struct sockaddr_in {
    sa_family_t sin_family; // AF_INET
    in_port_t sin_port;
    struct in_addr sin_addr;
};
struct in_addr {
    uint32_t s_addr;
};
```

## 1.2 阻塞模型
https://www.jianshu.com/p/ed1f9e9a1982
### 一 阻塞IO
```c
app                     kernel
+-----------------------+---------------------------
| 1. fd = accept()  --> |
                        | 2. [fd, ]
| 3. read(fd)       --> |
                        | 4. if not fd.data: 阻塞
                        | 5. receive data
                        | 6. copy data
| <--           7. data |
```
### 二 非阻塞IO
```c
app                     kernel
+-----------------------+---------------------------
| 1. fd = accept()  --> |
                        | 2. [fd, ]
| 3. read(fd)       --> |
                        | 4. if not fd.data: return
| <--        5. no data |
| 6. read(fd)       --> |
                        | 7. if not fd.data: return
| <--        8. no data |
```
### 三 IO多路复用
```c
app                     kernel
+---------------------------+---------------------------
| 1. fd1 = accept()     --> |
| 2. fd2 = accept()     --> |
                            | 3. [..., fd1, ..., fd2, ...]
| 4. select([fd1, fd2]) --> |
                            | 5. if one fd receive data: return
| <--   6. [..., 1, ..., 0] |
| 7. read(fd1)          --> |
| <--               8. data |
```
### 四 信号驱动IO
### 五 异步IO
```c
app                     kernel
+-----------------------+---------------------------
| 1. fd = accept()  --> |
                        | 2. [fd, ]
| 3. aio_read(fd)   --> |
                        | 4. if not fd.data: return
| <--        5. no data |
                        | 6. signal(app)    
| 7. signal_handle()    |
```

## 1.3 IO多路复用
函数：
- select()：一次调用，检查所有文件句柄，返回活跃的文件句柄
- poll()：一次调用，检查所有文件句柄，返回活跃的文件句柄
- epoll：文件句柄绑定回调函数，活跃的文件句柄自动调用回调函数
### 1.3.1 select()
```c
// 内核
FDS[] = [fd_1, fd_2, fd_3, fd_4, fd_5, ...]
FDSET = [false, false, false, false, false, ...]

// 内核接口：遍历FDS，返回哪些FDS接收到了数据
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

// 内核实现, fd_num最大为100
int select(maxfd, readfd, NULL, NULL, NULL)
{
    int num = 0;

    for (int i = 0; i < maxfd; i++) { /* 阻塞与非阻塞 */
        if (readfds[i] == 1) {
            readfds[i] = len(FDS[i].data) > 0 ? 1 : 0;
            num += len(FDS[i].data) > 0 ? 1 : 0;
        }
    }
        
    return num;
}

// 用户调用
void fetch_data()
{
    fd_set rfds;
    
    int fd1 = accept();
    int fd2 = accept();
    int fd3 = accept();

    for (;;) {
        FD_ZERO(&rfds);
        FD_SET(fd1, &rfds);
        FD_SET(fd2, &rfds);
        FD_SET(fd3, &rfds);

        select(fd3 + 1, &rfds, NULL, NULL, NULL); /* block here */

        if (FD_ISSET(fd1, &rfds)) {
            read(fd1, ...);
        }
        if (FD_ISSET(fd2, &rfds)) {
            read(fd2, ...);
        }
    }
}
```

### 1.3.2 poll()
### 1.3.3 epoll()
```c
// 用户调用
````

**epoll工作模式**
场景：
t1:
    buff = [xxxxxxx_______]
t2:
    read(buff)
t3:
    buff = [xxxx__________]

分类：
- level-trigger: t3 可读
- edge-trigger: t3 不可读
    while(buff.len > 0) {
        read(buff)
    }