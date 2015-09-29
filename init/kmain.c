/***************************************************************************
 *
 *  kmain.c-内核入口函数
 *  在这里进行内核参数的初始化工作
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "type.h"
#include "const.h"
#include "kernel.h"
#include "blk.h"
#include "fs.h"
#include "interrupt.h"
#include "memory.h"
#include "process.h"
#include "sched.h"
#include "time.h"
#include "tty.h"
#include "io.h"
#include "stdcall.h"
#include "buffer.h"

/*BIOS函数获得的扩展内存大小（1MB以上内存大小，以KB为单位）*/
#define EXT_MEMORY          (*(u16 *)(0x90000))
#define HD_DRIVER_INFO      ((void *)0x90020)
#define STARTUP_ROOT_DEV    (0x301)

/*连接时自动生成的符号*/
extern int end;
/*定义在kernel.asm中*/
extern void load_seg();

/*
*要用到的两个系统调用
*把函数定义成inline形式，这样就不会发生函数调用而使用堆栈了
*进入系统堆栈后，使用的是系统堆栈，这不会受影响
*因为CPU每次切换任务，进程的系统堆栈会被丢弃
*
*注意：表达式的末尾是不加分号的，请参见宏定义
*/
static inline syscall0(int,fork)
static inline syscall0(int,pause)

/*pre declaration*/
private void process1();

public void kmain(void)
{
    /*起动盘*/
    sys_root_dev = STARTUP_ROOT_DEV;
    
    /*硬盘信息首地址*/
    sys_hd_info = HD_DRIVER_INFO;
    
    /*物理内存*/
    sys_memory_start = 0;
    sys_memory_end = (1 << 20) + (EXT_MEMORY << 10);    /*字节*/
    sys_memory_end &= 0xFFFFF000;                       /*忽略不到4KB的内存*/
    
    if(sys_memory_end > 64 * MB)                        /*只支持64MB内存*/
    {
        sys_memory_end = 64 * MB;
    }
    
    /*高速缓存起始地址在内核代码末端*/
    buffer_memory_start = (end + 0xFFF) & 0xFFFFF000;
    
    /*如果内存大于16MB，则设缓冲区大小为4MB*/
    if(sys_memory_end > 16 * MB)
    {
        buffer_memory_end = 4 * MB;
    }
    
    /*如果内存大于8MB，但小于16MB，则设缓冲区末端为2MB*/
    if(sys_memory_end > 8 * MB)
    {
        buffer_memory_end = 2 * MB;
    }
    else
    {
        buffer_memory_end = 1 * MB;
    }
    
    /*内存区起始位置*/
    main_memory_start = buffer_memory_end;
    main_memory_end = sys_memory_end;
    
    /*
    *下面开始顺序调用系统各模块的初始化函数
    */
    mem_init();
    int_init();
    init_8259A();
    syscall_init();
    blk_dev_init();
    init_tty();
    timer_init();
    sched_init();
    mem_buffer_init();
    hd_init(sys_hd_info);
    /*开启中断*/
    sti();
    
    /*下面手动执行第一个进程，首先在GDT中设置任务0*/
    set_ldt_desc(sys_gdt + INDEX_GDT_LDT,24,&process[0]->ldt[INDEX_USER_CS]);
    set_tss_desc(sys_gdt + INDEX_GDT_LDT,104,&process[0]->tss);
    load_ldt(TASK_LDT);
    load_tr(TASK_TSS);
    
    /*
    *将代码流的执行移动到用户态
    *下面将要调用的函数是用#define定义的
    */
    move_to_user();
    
    /*下面正式开始进程*/
    if(!fork())
    {
        process1();
    }
    
    /*如果执行到这里，说明系统中只剩下进程0了，系统进入中断循环状态*/
    while(TRUE)
    {
        pause();
    }
}

/*
*进程1执行的函数
*在这里将要开启一个shell，系统就正式开始可用了
*/
private void process1()
{
    /*重新加载数据寄存器*/
    load_seg();
}
