

# 1 概述
## 1.1 场景
## 1.2 通信方式
- 信号
- 信号量 （锁）
- 消息队列 （传递数据）
- 管道
- 共享内存
- socket

# 2 使用方法
## 2.1 代码
### 信号 
```c
// 发送
kill(pid_t pid, int signum)
// 接收
signal(int signum, sig_handle_func()) /* 不阻塞，先恢复信号，再处理 */
sigaction(int signum, ...) /* 阻塞，先处理，再恢复信号 */
```
### 信号量 （Posix）
```c
#include <semaphore.h>
// 初始化
int sem_init(sem_t *sem, int pshared, unsigned int value); /* 0：进程内线程同步 */
int sem_destroy(sem_t *sem);
// 上锁
int sem_wait(sem_t *sem);
// 解锁
int sem_post(sem_t *sem);
// 有名信号量
sem_t *sem_open(const char *name, int flag);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
// 其他
int sem_trywait(sem_t *sem);
int sem_timewait(sem_t *sem);
```
### 消息队列
```c
// 初始化
int msgget(key_t key, int msgflg); /* flag: 访问权限 */
key_t ftok(const char *pathname int projid);
// 发送
int msgsnd(int msqid, const void *data, size_t datasz, int msgflag);
// 接收
ssize_t msgrcv(int msqid, void *data, size_t datasz, long msgtype, int msgflag); /* msgtype: 接收指定类型消息 */
// 删除、赋值、查询
msgctl(int msqid, int cmd, struct msqid_ds *buf);
```
### 管道
```c
```

## 2.2 命令
```bash
# 信号: 查看
kill -l
```