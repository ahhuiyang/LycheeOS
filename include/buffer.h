/***************************************************************************
 *
 *  fs.h-文件系统常量和结构定义
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/

#ifndef _BUFFER_H
#define _BUFFER_H

#include "kernel.h"
#include "process.h"

/*内存缓冲区数据结构，指向一内存缓冲块*/
typedef struct _struct_mem_buffer struct_mem_buffer;

struct _struct_mem_buffer
{
    char *data;                 /*指向数据块（1024B）的指针*/
    unsigned int block;         /*块号，从0开始*/
    unsigned int dev;           /*设备编号，加上块号就可寻址了*/
    unsigned char uptodate;     /*缓冲块中的数据是有效的*/
    unsigned char dirty;        /*缓冲块中的数据与设备中的数据不一致*/
    unsigned char count;        /*使用此缓冲块的进程数*/ 
    unsigned char locked;       /*是否加锁*/
    struct_process *proc_wait;   /*等待此内存缓冲块的进程头指针*/
    struct_mem_buffer *prev;    /*双向链表*/
    struct_mem_buffer *next;    /*双向链表*/
};

/*以下函数定义在buffer.c中*/
public void mem_buffer_init();
public struct_mem_buffer *bread(int dev,int block);
public void brelease(struct_mem_buffer *mbuffer);
public struct_mem_buffer *get_buffer(int dev,int block);
public struct_mem_buffer *find_buffer(int dev,int block);
public void invalid_dev_buffers(int dev);
public void sync_dev(int dev);

#endif
