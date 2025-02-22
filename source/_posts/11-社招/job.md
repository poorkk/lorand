---
title: z 社招 各科目知识点
date: 2022-09-25 16:03:37
categories:
    - 社招
tags:
    - 社招
---

[toc]

## 一、能力范围
### 1 操作系统
#### CPU体系结构 - ok
- CPU
    - 构成
        - 运算器
        - 控制器
        - 寄存器：控制寄存器、状态寄存器
        - 高速缓存
    - 寄存器（8086 14个寄存器） https://zhuanlan.zhihu.com/p/115915986
      ```bash
      +--------------+  <-- SS 栈寄存器       
      | Stack        |                          | AX BX CX DX 通用寄存器
      +--------------+      <-- SP 栈顶寄存器
      |.             |      <-- BP 基指针寄存器
      |.             |
      +--------------+
      | Heap         |                          | SI DI 源变址寄存器、目标变址寄存器
      +--------------+
      | Data Segment |
      +--------------+  <-- DS 数据段寄存器
      | Code Segment |      <-- IP 指令寄存器    | 
      +--------------+  <-- CS 代码段寄存器
      |...           |
      +--------------+  <-- ES 附加段寄存器
      | Kernel Stack |                           | FLAG 标志寄存器
      +--------------+
      | Kernel Code  |
      +--------------+
      ```
    - CPU寻址方式
        - 直接寻址 : [1]
        - 寄存器间接寻址 [bx]
        - 寄存器相对寻址 [bx + 1]
        - 基址变址寻址 [bx + si]
        - 相对基址变址寻址 [bx + si + 1]
    - CPU特权级别（x86架构支持4个cpu特权级别，不同级别支持的指令集不同）
        - R0：内核态
        - R1:
        - R2:
        - R3: 用户态
    - CPU执行状态
        - 状态分类
            - 内核态：操作系统程序
            - 用户态：用户程序
        - 状态转换
            - 转换类型
                - 用户态 -> 内核态：中断，异常，陷入（操作系统提供给用户的接口）
                - 内核态 -> 用户态：设置程序状态字PSW
            - 中断：来自IO设备、其他硬件设备（硬件异步）
                - 分类
                    - IO中断
                    - 时钟中断
                    - 硬件故障
                - 中断行为：保存当前进度，处理中断，返回下一条指令
                    1. cpu 取下一条指令
                    2. cpu 执行指令
                    3. cpu 执行一条指令后，扫描中断寄存器，查看是否有中断信号
                    4. 如果有中断，中断硬件保存现场
                    5. 中断硬件将触发器的内容送入PSW对应位置，查找中断向量表，调用中断处理程序
                - 中断向量表
                    - 0-19：不可屏蔽中断和异常
                        - 0 除0
                        - 1 单步调试
                        - 4 算数溢出
                        - 6 非法操作数
                        - 12 栈异常
                        - 13 保护性错误
                        - 14 缺页异常
                    - 20-31：保留
                    - 32-127：外部中断
                    - 128：系统调用异常 <-- （重要）
                    - 129-238：外部中断
                    - 239：本地APIC时钟中断
                    - ...
                    - 255 本地APIC伪中断
                - 多核CPU处理中断
                    - 中断控制器 GIC Generic Interrupt Controller：全局中断的分发与管理，判断中断交给哪个CPU
            - 异常（软件同步）
                - 分类
                    - 陷入：有意识安排，返回下一条指令：系统调用
                    - 故障：可恢复错误，返回当前指令：缺页异常、段错误
                    - 终止：不可恢复错误、算术溢出、除0、...
                - 异常处理
                    - 1.进入异常时：
                        - 保存处理器状态
                        - 准备高特权级执行环境
                        - 选择并执行合适的异常处理代码
                    - 2.处理异常时：
                        - 获取异常信息（调用参数、原因等）
                    - 3.结束异常时：
                        - 恢复处理器状态
                        - 返回低优先级
                        - 继续执行流
        - CPU指令类型
            - 特权指令：启动IO、内存清0、修改程序状态字、设置时钟、允许/禁止中断
            - 非特权指令：控制转移、算数运算、取数指令 
#### 内存管理 - ok
- 分段
    - 设计
        - 虚拟地址分为若干大小的段
        - 段表存储分段信息
        - 每个进程1个段表，操作系统1个全局段表
        ```bash
        虚拟地址               段表                              进程
        +-----------+         +--------+--------+--------+      +------------+
        | Segment 0 |         | 段号   | 段基址  | 段长度 |       | Process 0 |
        |           |         +--------+--------+--------+      +------------+
        +-----------+         | 0      | 0xxxx  | 2k     |
        | Segment 1 |         | 1      | 0xxxx  | 4k     |
        |           |         | 3      | ..     | ..     |
        |           |
        |           |
        +-----------+
        ```
    - 实现
        - 虚拟地址为：[段号 + 段内偏移]（段号->段基址 + 段内偏移）
        - 物理地址：以段为单位分配
    - 优点
        - 逻辑连续
    - 缺点
        - 分段粒度粗，容易产生外部碎片
- 分页
    - 设计
        - 物理内存划分为连续、等长的物理页
        - 虚拟页与物理页大小相等
        - 虚拟页可映射至任意物理页
        - 每个进程1个页表（或1级页表）
    - 实现
        - [虚拟页号 + 页内偏移] 

        ```bash
        - 页表
        --------+----------
        虚拟页号 | 物理页号
        --------+----------
        0       |    10
        1       |    13
        --------+----------
        ```
    - 优点
        - 解决外部碎片
        - 应用程序时空局限性：执行时，使用的数据或代码是连续的
    - 缺点
        - 页表过大（解决：多级页表 -> 剪枝）
            - 多级页表：[虚拟页号_0:虚拟页号_1:虚拟页号_2:虚拟页号_3 + 页内偏移]

    - 优化
        - TLB （性能）
            - 位置：位于CPU内部
            - 功能：缓存虚拟页号到物理页号的映射关系
            - 作用：先查TLB，如果TLB未命中，再查缓存
            - 刷新：切换页表时，刷新全部TLB（优化：TLB缓存项记录页表标签，切换页表时无需全部刷新）
            
            ```bash
            [CPU] --> [TLB] --> [页表]
                    [p3]
                    [p9]
                    [p5]
            ```
            - 结构：根据CPU缓存 L1 L2 L3 分级结构 -> TLB 也是分级结构
        - swapping （大小）
            - 功能：虚拟内存 > 物理内存时，将内存页存放到磁盘
            - 算法：FIFO、LRU/MRU、Clock
        - 缺页异常 （稳定）
            - 原因：物理内存需求过大
            - 处理：等待磁盘IO，将物理页读取至内存
        - 伙伴系统（利用率）
            - 问题：外部碎片：空闲但不连续；内部碎片：分配空间过大
            - 实现：分裂与合并

            ```bash
            - 伙伴系统
            时间1：    [                    ]
            
            时间2：    [         ] [        ]
            
            时间3：    [   ] [   ] 
            ```

            - 空闲链表：较大空间分裂时，将多余的部分移入上一空间数组链表
            ```bash
            - 空闲数组链表
            [ 2^0 ]
            [ 2^1 ] -> [] -> []
            [ 2^2 ] -> []
            ```
        - 写时复制（共享）
#### 进程线程架构 - ok
- 进程
  - 定义：Process/Task/Job 定义：某个数据集合上一次运行活动。资源分配单位。
  - 实现：进程控制块 Process Control Block PCB：进程描述符，进程属性
    ```bash
    +---------+
    | PCB 0   |          PCB 内容
    | PCB 1   | --> +---------------+
    | PCB 2   |     | pid           | 1. 基本信息：pid, 进程名，userid
    | PCB 3   |     | cpu register  | 2. CPU现场：寄存器，程序计数器，状态字PSW，栈指针，页表指针
    | PCB ..  |     | process state | 3. 控制信息：状态，优先级，代码执行入口，程序磁盘地址，运行统计信息（执行时间，页面调度），进程同步与通信，进程队列指针，进程消息队列指针
    +---------+     | resource      | 4. 资源信息：虚拟地址空间，打开的文件
                    +---------------+
    ```
  - 状态：
    - 分类：
      - 就绪态：CPU不空闲
        - -> 运行：被调度程序选中
      - 运行态：
        - -> 就绪：时间片用完，高优先级抢占
        - -> 等待：系统调用、IO、进程通信、...
      - 等待态：阻塞
        - -> 就绪：等待事件发生
      - 创建态：
      - 终止态：进程结束，统计运行信息，资源回收
      - 挂起态：调节负载）（进程映像交换到磁盘）
    - linux 进程状态：
      ```c
      fork()  ->  就绪态  ->  运行态  +-->  浅度睡眠 --+
                                    |               |
                                    +-->  深度睡眠 --+
                                    |               |--> 就绪态 （wake_up / 信号）
                                    +--> 暂停     --+
                                    |              |
                                    +--> 僵尸     --+
      就绪->运行：schedule()
      运行->就绪：时间片结束
      运行->浅度睡眠：等待资源，interruptible_sleep_on()，schedule()
      运行->深度睡眠：等待资源，sleep_on()，schedule()
      运行->暂停：ptrace调试，schedule()
      ```
  - 进程队列
    进程状态改变：从一个队列出队，进入另一个队列
    等待队列：等待事件不同，有多个等待队列
  - 进程生命周期
    - 创建 fork/exec：分配pid，分配pcb，复制父进程地址空间（linux优化：当子进程要写父进程地址时，才复制地址空间），继承父进程共享资源(打开的文件，工作目录)，设置队列指针
    - 阻塞 wait 等待子进程：自己执行阻塞原语
    - 唤醒
    - 挂起
    - 激活
    - 撤销 exit：关闭文件，回收内存，撤销pcb
  - 进程切换
    - 进程地址空间
      ```c
      --------- 用户地址空间
      栈
      .
      .
      .
      堆
      ---------
      数据段
      ---------
      代码段
      ---------
      ...
      ----------- 操作系统地址空间
      内核栈
      -----------
      内核代码
      ------------
      ```
    - 上下文切换：将cpu硬件状态从一个进程换到另一个进程
      - 直接开销：保护和恢复寄存器、切换地址空间
      - 间接开销：高速缓存失效、缓冲区缓存失效、TLB失效
  - 进程通信
    - 信号
    - 信号量 （锁）
    - 消息队列 （传递数据 + 阻塞）
    - 管道 （传递数据 + 阻塞）
    - 共享内存 （传递数据 + 阻塞）
    - 套接字 （传递数据 + 阻塞）

- 线程
  - 进程问题
    - 创建进程开销大：数据段、代码段、堆、栈段
    - 进程隔离性强：IPC开销大
    - 进程内部无法支持并行
  - 线程优点
    - 构成：执行所需最小状态：寄存器和栈（每个线程有自己的栈，共享数据、代码、堆）
      - 线程id，线程状态，线程上下文，线程栈和栈指针
    - 开销小：创建和销毁，切换，通信
    - 性能高：多处理器并发执行
  
  - 线程模型（用户线程 与 内核线程）
    - 多对一
    - 一对一：内核线程数量大
    - 多对多
  - 线程调度
    - 调度场景
      ```bash
      [t1]                [CPU1]
      [t2] -> [调度器] ->  
      [t3]                [CPU2]
      ```
    - 调度算法
      - 衡量标准
        - 吞吐量
        - 周转时间：请求 -> 运行
        - 响应时间：请求 -> 第一次回应

        - CPU利用率
        - 等待时间：就绪 -> 运行
      - 算法分类
        - 算法类型
          | 算法 | 实现 | 优点 | 缺点 |
          |-|-|-|-|
          批处理系统 |
          先来先服务 FCFS | - | 公平、简单 | 长任务不友好
          最短作业优先 SJF | - | 最短平均周转 | 长任务不友好
          最短剩余时间优先 SRTN | 
          最高响应比优先 HRRN | 响应时间 = （处理时间 + 等待时间） / 处理时间 
          交互式系统 |
          时间片轮转 RR | - | 公平、响应快 | 进程切换开销高、IO进程不友好 （IO进程进入优先级高、独立就绪队列）
          最高优先级 HPF | 优先级动态或静态 | - | 低优先级饥饿、优先级反转（低优先级进程持有锁，高优先级进程抢占，低优先级无法执行）
          多级反馈队列 MFQ | 多个就绪队列，优先级依次降低，时间片依次增加，时间片轮转调度，高优先级优先调度
          最短进程优先 |
        - 操作系统
          | OS | 策略 |
          |-|-|
          unix | 动态优先
          BSD | 多级反馈
          Linux | 抢占
          Windows | 32优先级抢占（优先级进入就绪队列）
          Solaris | 综合调度
    - 调度问题
      - 优先级反转
        使用临界区资源时：
        - 优先级上限：优先级临时设为最高
        - 优先级继承：临时继承最高优先级
        - 禁止中断：禁止被抢占
  - 线程通信
    - 临界区
      - 要求
        - 互斥访问
        - 有限等待
        - 空闲让进
      - 算法
        - 皮特森算法 (?)
          ```c
          // 进程 0
          while (1) {
            flag[0] = true;
            turn = 1;
            while (flag[1] == true && turn == 1) {
              /* 临界区 */
              flag[0] = false;
            }
          }

          // 进程 1
          while (1) {
            flag[1] = true;
            turn = 0;
            while (flag[0] == true && turn == 0) {
              /* 临界区 */
              flag[1] = false;
            }
          }
          ```
        - 硬件原子
          - 概念
            - 不可被打断的操作集合
            - 其他核心不会看到中间状态
          - 实现
            - intel 锁总线
            - arm LL/SC
    - 线程锁
      - 自旋锁
      - 读写锁
    - 线程死锁
      - 预防
        - 避免互斥访问
        - 不允许持有并等待
        - 不允许资源抢占
        - 打破循环等待（按特定顺序获取资源）
      - 解决
        - 银行家算法
          - 获取资源需管理者同意
          - 管理者预演不会造成死锁
  - 多核缓存
    ```bash
    [  cpu 0 ] [  cpu 1 ]      [  cpu 2 ] [  cpu 3 ]
    [l1 cache] [l1 cache]      [l1 cache] [l1 cache]

    [     l2 cache      ]      [      l2 cache     ]

    [                  l3 cache                    ]

    [                     内存                     ]
    ```
    - 缓存一致性
      - 原则
        - 独占修改
          - cpu核独占缓存行：本地可读可写
          - 其他核读需迁移到共享
          - 其他核写需迁移到失效
        - 共享
          - 多核拷贝缓存行
          - 本地可读
          - 本地写迁移到独占修改，使其他核缓存行失效
          - 其他核写需迁移到失效
        - 失效
          - 本地缓存行失效
          - 本地不能读/写缓存行
          - 本地读需迁移到共享，其他核需迁移到共享
          - 本地写需迁移到独占修改，其他核该行失效
      - 通知其他核
        ```bash
                    全局缓存：记录缓存行拥有者信息
                      | dirty cpu1 cpu2 cpu3 |
                      |0     1    1    1     |

        | 状态 内容 |     | 状态 内容 |     | 状态 内容 |
            cpu1              cpu2             cpu3
        ```
#### IO管理
#### 文件管理系统

### 2 计算机网络
#### 应用层

#### 传输层
#### 网络层
#### 链路层
#### socket编程

### 3 编程语言
#### C语言
#### 内核代码
#### python

### 4 数据结构
#### 栈、队列
#### 树 - ok
- **B树**：一个节点存多个值
  - 高效：降低树高度，结和IO机制，降低IO次数
- **B+树**：只有叶子节点存数据
  - 高效：中间节点不存数据，可存更多节点
  - 稳定：查找次数一致
  - 有序：叶子节点组成链表，方便批量查询
- **B-Link树**：节点指向右兄弟；每个节点存储High-key
  - 并发：根据High-key，并发场景下，可判断节点是否发生分裂，无需加锁
#### 图
#### 算法 - ok
- 分治
- 动态规划：用表记录所有结果，避免重复计算（斐波那契、最长公共子序列）
- 贪心：优先满足性价比高的（0-1背包、找零等问题）
- 回溯：枚举 + 剪枝

### 5 密码学

### 6 网络安全



### 7 其他
#### 计算机体系结构
#### 汇编原理
#### 编译原理

## 二、工作经历
### 1 职责
### 2 模块
#### SSL通信
#### 身份认证
#### 访问控制
#### 脱敏
#### 防篡改
#### 统一审计
### 3 能力
#### 数据库整体架构
#### SQL引擎
#### 存储引擎
#### 公共模块

## 三、项目经历
### 透明加密
### 密态
### 防火墙
### 网盘
### 新闻客户端

## 四、实习经历
### 爬虫

## 五、技能爱好
### 工具
### 书籍