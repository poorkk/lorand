#include <stdio.h>
#include <stdlib.h>

/* 指针： 段号10 页号10 页内偏移12 */

/* https://blog.csdn.net/jinking01/article/details/107098437 */

/* 
 * 每次申请，都是16的倍数
 *
 * 指针尾部8字节 [ n * 16 ][ 8 ]
 * 
 * 申请大小     实际大小        构成 
 * 1-24         32          [xxxxxxxx][xx                 ] 头部8字节，前4字节存储大小，和pg内存上下文几乎一样实现
 *                                     ^
 *                                   return
 * 25-40        48
 * 41-56        64  
 */
#define SEG_NO_MASK   0xffc00000
#define PAGE_NO_MASK  0x003ff000
#define PAGE_OFF_MASK 0x00000fff

#define ptr_get_segno(ptr) ((unsigned int)(ptr) &  (unsigned int)SEG_NO_MASK)
#define ptr_get_pageno(ptr) ((unsigned int)(ptr) &  (unsigned int)PAGE_NO_MASK)
#define ptr_get_pageoff(ptr) ((unsigned int)(ptr) &  (unsigned int)PAGE_OFF_MASK)

void prt_ptr(void *ptr)
{
    printf("%-8p  | %-8p  | %d\n", ptr_get_segno(ptr), ptr_get_pageno(ptr), ptr_get_pageoff(ptr));
}

void _prt_mem_addr()
{
#define CNT 40
    void *ptr[CNT];
    int i;
    //char *a = (char *)malloc(32);
    for (i = 0; i < CNT; i++) {
        ptr[i] = (void *)malloc(i * 16);
        memset(ptr[i], 'a', i * 16);
    }

    for (i = 0; i < CNT; i++) {
        prt_ptr(ptr[i]);
    }

    for (i = 0; i < CNT; i++) {
        printf("%u\n", *((unsigned int *)(ptr[i] - 8)));
    }

    for (i = 1; i < CNT; i++) {
        printf("    %lu ", ptr[i] - ptr[i - 1]);
    }
    printf("\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() 
{
    // generate key
    key_t key = ftok("./21", 200);
    printf("key=%#x\n", key);

    // create a share memory
    int shmid = shmget(key, 400, IPC_CREAT|0666|IPC_EXCL);
    if(shmid == -1)
    {
        perror("shmget 0 failed\n");
        exit(1);
    }
    printf("shmid=%#x\n", shmid);

    // map share memory to get the virtual address
    void *p = shmat(shmid, 0, 0);
    if((void *)-1 == p)
    {
        perror("shmat failed");
        exit(2);
    }

    key_t key1 = ftok("./22", 200);
    int shmid1 = shmget(key1, 400, IPC_CREAT|0666|IPC_EXCL);
    if(shmid1 == -1)
    {
        perror("shmget 1 failed\n");
        exit(1);
    }
    void *p1 = shmat(shmid1, 0, 0);

    prt_ptr(p);
    prt_ptr(p1);

    // write data to share memory
    int *pi = p;
    *pi = 0xaaaaaaaa;
    *(pi+1) = 0x55555555;

    // remove the map
    if(shmdt(p) == -1)
    {
        perror("shmdt failed");
        exit(3);
    }

    // delete the share memory
    printf("use Enter to destroy the share memory\n");
    getchar();
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(4);
    }
    return 0;
}