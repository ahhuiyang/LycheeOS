/***************************************************************************
 *
 *  process.h-进程及进程相关数据结构
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _PROCESS_H
#define _PROCESS_H

#include "type.h"
#include "tty.h"
#include "fs.h"
#include "signal.h"
#include "kernel.h"

/*进程的状态*/
#define PROCESS_RUNNING                 0 /*所有可以运行的进程，只有一个在CPU上执行*/
#define PROCESS_WAIT_INTERRUPTIBLE      1 /*可中断等待状态，可以被信号激活*/
#define PROCESS_WAIT_UNINTERRUPTIBLE    2 /*不可中断等待状态，即信号无法激活处于此状态的进程*/
#define PROCESS_STOPPED                 3 /*中止状态，指进程已死*/

/*tss的定义 size=104 bytes*/
struct _struct_tss
{
    u16 pre_tss_selector;
    u16 reserved1;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt_selector;
    u16 tflag;
    u16 io_base;
};

/*进程内核控制块*/
struct _struct_process
{
    unsigned int state;         /*进程状态*/
    
    unsigned int ticks;         /*进程剩余的时间片数，0-15*/
    unsigned int priority;      /*进程优先级，在调度时使用，0-31*/
    
    /*以下用于信号处理，从linux引进，暂时放在这里，以后再处理*/
    unsigned int signal;        /*进程收到的信号*/
    struct_sigaction sigaction[32];     /*信号处理过程*/
    unsigned int sig_blocked;   /*用于屏蔽信号*/
    
    int exit_code;
    
    int start_code,end_code,end_data,end_bss,start_stack;
    
    unsigned int pid;           /*进程标识号*/
    unsigned int ppid;          /*父进程标识号*/
    unsigned int user_time;     /*进程在用户态运行的时间*/
    unsigned int kernel_time;   /*进程在核心态运行的时间*/
    unsigned int create_time;   /*进程开始运行的时间*/
    unsigned int exit_time;     /*进程退出时间*/
    struct_tty *tty;            /*当前进程使用的tty*/
    
    /*文件系统*/
    struct_mem_inode *pwd;      /*工作目录*/
    struct_mem_inode *root;     /*文件系统根目录*/
    struct_mem_inode *image;    /*可执行文件i节点*/
    
    struct_file *files[NR_OPEN]; /*打开的文件数组*/
    
    /*以下字段用于内核*/
    struct_descriptor ldt[3];   /*0-cs,1-ds and ss*/
    struct_tss tss;             /*tss for this process*/
};

#endif
