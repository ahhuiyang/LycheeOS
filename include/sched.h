/***************************************************************************
 *
 *  sched.h-进程调度及相关
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _SCHED_H
#define _SCHED_H

/*下面的宏将系统由用户模式移动到内核模工*/
#define move_to_user()\
__asm__ ("movl %%esp,%%eax\n\t"\
         "pushl $0x17\n\t"\
         "pushl %%eax\n\t"\
         "pushfl\n\t"\
         "pushl 0x0F\n\t"\
         "pushl restart\n\t"\
         "iret\n\t"\
         "restart:\tmovl $0x17,%%eax\n\t"\
         "movw %%ax,%%ds\n\t"\
         "movw %%ax,%%es\n\t"\
         "movw %%ax,%%fs\n\t"\
         "movw %%ax,%%gs\n\t"\
         :::"ax")
         
#endif
