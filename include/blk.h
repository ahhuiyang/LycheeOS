/***************************************************************************
 *
 *  blk.h-块设备常量定义
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _BLK_H
#define _BLK_H

#include "type.h"
#include "process.h"
#include "fs.h"

/*读写标识*/
#define READ    0
#define WRITE   1

/*现在只有一个块设备，以后扩展*/
#define NR_BLK_DEVICE   7   /*硬盘，...*/
#define NR_REQUEST      32  /*请求项数*/

/*主设备号*/
#define MAJOR_NULL      0
#define MAJOR_MEM       1
#define MAJOR_USB       2
#define MAJOR_HD        3
#define MAJOR_TTYS      4
#define MAJOR_TTY       5
#define MAJOR_LP        6
#define MAJOR_PIPE      7

/*数据是否有效标志*/
#define DATA_NO         0
#define DATA_YES        1

/*一磁盘块或逻辑块对应多少个扇区*/
#define BLOCK_SIZE      1024
#define BLOCK_SIZE_BITS 10
#define SECTOR_SIZE     512

#define MAJOR(dev) ((dev) >> 8)
#define MINOR(dev) ((dev) & 0xFF)

/*请求项，每一次对硬盘的操作都转化成请求队列中的一项，然后有专门的函数处理此请求项*/
typedef struct _struct_request
{
    int dev;                        /*-1 if no request*/
    int cmd;                        /*读或写*/
    int nr_errors;                  /*读写发生错误的次数*/
    unsigned int start_sector;      /*起使扇区，相对于当前分区的序号，不能大于分区总扇区数*/
    unsigned int nr_sectors;        /*读写扇区总数*/
    char *buffer;                   /*指向mbuffer->data*/
    struct_process *proc_wait;      /*等待操作完成的地方*/
    struct_mem_buffer *mbuffer;     /*高速缓冲指针*/
    struct _struct_request *next;   /*指向下一请求项*/
}struct_request;

/*每一个块设备都有一个块设备结构，指明其请求和操作函数*/
typedef struct _struct_blk_dev
{
    void (*do_request_fn)(void);    /*处理请求项的函数*/
    struct_request *current_request;/*当前处理请求项*/
}struct_blk_dev;

/*声明在blk.c中*/
public extern struct_blk_dev blk_dev[NR_BLK_DEVICE];
public extern struct_request blk_request[NR_REQUEST];

#endif
