/***************************************************************************
 *
 *  memory.h-与内存管理有关的常数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _MEMORY_H
#define _MEMORY_H

#include "type.h"
#include "process.h"

#define PAGE_SIZE 4096          /*4096B = 4KB*/
#define MEM_BUFFER_SIZE 1024    /*1024B = 1KB*/

#define B       1
#define KB      (1024 * B)
#define MB      (1024 * KB)
#define GB      (1024 * MB)

/*页目录和页表位*/
#define PD_P        0x1         /*present,0-不在内存，1-在内存*/
#define PD_RW       0x2         /*read/write,0-只读，1-读写*/
#define PD_US       0x4         /*user/supervisor，特权级*/
#define PD_PWT      0x8         /*控制缓冲策略*/
#define PD_PCD      0x10        /*0-页面可以缓冲，1-不可*/
#define PD_A        0x20        /*Accessed*/
#define PD_PS       0x80        /*page size,0-4KB,1-byte*/
#define PD_G        0x100       /*global page，即使tlb需要刷新，指定页表也不会变为无效*/
#define PT_P        PD_P
#define PT_RW       PD_RW
#define PT_US       PD_US
#define PT_PWT      PD_PWT
#define PT_PCD      PD_PCD
#define PT_A        PD_A
#define PT_D        0x40        /*Dirty，页面或页表修改标志*/
#define PT_DAT      0x80        /*page table attribute index*/
#define PT_G        PD_G

#endif
