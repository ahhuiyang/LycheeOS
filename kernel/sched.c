/***************************************************************************
 *
 *  schedule.c-跟进程调度有关的函数及数据
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "process.h"
#include "memory.h"
#include "type.h"
#include "interrupt.h"

/*包括系统堆栈*/
union union_process
{
    struct_process process;
    /*用于系统堆栈*/
    char kstack[PAGE_SIZE];
};

/*全局变量，自开机以来系统滴答数(10ms/滴答)*/
unsigned int sys_ticks;
struct_process *current;    /*当前任务指针*/
/*进程指针数组*/
struct_process *process[NR_PROCESS];

/*系统堆栈，一个页面大小*/
public int sys_stack[1024];

/*
*   进程调度函数
*   根据进程的优先级和剩余时间片
*   剩余时间越多，优先级越高的进程越先执行
*/
public void schedule()
{
    int i,hit,max;
    struct_process **p;
    
    /*循环，直到找到一个可以调度的进程*/
    while(TRUE)
    {
        /*进程的剩余时间片有可能为0*/
        max = -1;
        p = &process[NR_PROCESS - 1];
        
        /*循环查找剩余时间片最大的进程*/
        for(i = 0 ; i < NR_PROCESS ; i ++)
        {
            /*如果进程为空，则继续下一个进程*/
            if(!process[i])
            {
                continue;
            }
            
            /*如果进程的剩余时间片比当前最大的剩余时间片大，则更新最大时间*/
            if(process[i]->state == PROCESS_RUNNING
                && process[i]->ticks > max)
            {
                    max = process[i]->ticks;
                    hit = i;
            }   
        }
        
        /*
        *如果存在某个进程的剩余时间片大于0，则退出循环
        *注意，在整个循环中，只有这一处出口
        */
        if(max) break;
        
        /*
        *否则，说明全部进程的剩余时间片为0
        *则根据每个进程的优先级，更新每一个进程的ticks
        *计算公式为：ticks = ticks / 2 + priority
        */
        for(i = 0 ; i < NR_PROCESS ; i ++)
        {
            if(process[i])
            {
                process[i]->ticks = process[i]->ticks >> 1 
                    + process[i]->priority;
            }
        }
    } 
    
    /*切换到任务号为hit的任务运行*/
    switch_to(hit);
}

/*
*将任务置为不可中断的等待状态，直到被wake_up明确地唤醒
*这个函数会在多个进程之间形成一条等待链
*最后加入的进程，会放在这条等待链的顶端
*/
public void sleep_on(struct_process **proc_wait_head)
{
    struct_process *tmp;
    
    /*若指针中存放的进程指针的地址无效，则返回*/
    if(!proc_wait_head)
    {
        return;
    }
    
    /*变量tmp指向等待链头，若是第一次等待，则tmp应该空*/
    tmp = *proc_wait_head;
    
    /*等待链头替换为当前进程*/
    *proc_wait_head = current;
    
    /*把当前进程置为不可中断的睡眠状态*/
    current->state = PROCESS_WAIT_UNINTERRUPTIBLE;
    
    /*重新调度，让其它进程执行*/
    schedule();
    
    /*到这里，说明此进程已被唤醒，则将队列头移向等待链中的下一个进程*/
    *proc_wait_head = tmp;
    
    /*如果还存在等待任务，则将其置为就绪态*/
    if(tmp)
    {
        tmp->state = PROCESS_RUNNING;
    }
}

/*
*把当前任务置为可中断的等待状态
*处于可中断睡眠中的进程可以被信号唤醒
*不可中断睡眠在任何情况下都不能被唤醒，除非显示调用wake_up唤醒
*/
public void interruptible_sleep_on(struct_process **proc_wait_head)
{
    struct_process *tmp;
    
    /*如果参数为空，返回*/
    if(!proc_wait_head)
    {
        return;
    }
    
    /*等待链头替换为当前进程*/
    tmp = *proc_wait_head;
    *proc_wait_head = current;
    
    /*必须首先唤醒的是队列头*/
    while(TRUE)
    {
        /*置当前任务的状态为可中断的睡眠状态*/
        current->state = PROCESS_WAIT_INTERRUPTIBLE;
    
        /*重新调度*/
        schedule();
        
        /*如果唤醒的进程头是当前进程，则退出循环*/
        if(*proc_wait_head && *proc_wait_head == current)
        {
            break;
        }
        
        /*
        *否则，说明唤醒的进程是等待链中间的某进程
        *必须要求首先唤醒等待链头
        *置等待链头为可运行态，返回while处开始，置当前进程为可中断睡眠态
        */
        (*proc_wait_head)->state = PROCESS_RUNNING;
    }
    
    /*执行到这里，说明等待链头被唤醒了，则将链头指向等待链上下一个进程*/
    *proc_wait_head = tmp;
    
    /*其它任务也切换为可执行，因为等待条件满足了，所有等待的任务应具有同等的机会*/
    if(tmp)
    {
        tmp->state = PROCESS_RUNNING;
    }
}

/*唤醒等待链头*/
public void wake_up(struct_process *p)
{
    if(p)
    {
        p->state = PROCESS_RUNNING;
    }
}

/*时钟中断处理函数*/
public void irq_timer_handler(int cpl)
{
    /*当前进程的内核时间和用户时间*/
    if(cpl == 0)
    {
        current->kernel_time ++;
    }
    else
    {
        current->user_time ++;
    }
    
    /*
    *   当前进程的时间片减1
    *   如果进程的时间片不为零，则退出，当前进程继续执行
    */
    if((-- current->ticks) > 0) return;
    
    /*将进程的时间片重新调整为零*/
    current->ticks = 0;
    
    /*重新调度进程*/
    schedule();
}

/*
*系统调用
*取当前正在运行的进程号
*/
public int sys_getpid()
{
    return current->pid;
}

/*
*系统调用
*让一个进程进入可中断的睡眠状态
*/
public int sys_pause(void)
{
    current->state = PROCESS_RUNNING;
    schedule();
    return 0;
}

/*调度初始化函数，用于对任务和时钟的初始化*/
public void sched_init()
{
    /*初始化任务0*/
    process[0]->state = PROCESS_RUNNING;
    process[0]->ticks = 15;
    process[0]->priority = 15;
    process[0]->signal = 0;
    memset((char *)process[0]->sigaction,0,sizeof(struct_sigaction) * 32);
    process[0]->sig_blocked = 0;
    process[0]->exit_code = 0;
    process[0]->pid = 0;
    process[0]->user_time = 0;
    process[0]->kernel_time = 0;
    process[0]->create_time = CURRENT_TIME;
    process[0]->exit_time = 0;
    process[0]->tty = &tty_table[nr_current_tty];
    process[0]->pwd = NULL;
    process[0]->root = NULL;
    process[0]->image = NULL;
    
    set_descriptor(&process[0]->ldt[0],0,0,0,0,0,0);
    set_descriptor(&process[0]->ldt[1],0xA0,0,0,0x0A,0xC0,0);
    set_descriptor(&process[0]->ldt[2],0xA0,0,0,0x02,0xC0,0);
    memset(&process[0]->tss,0,sizeof(struct_tss));
    process[0]->tss.esp0 = PAGE_SIZE + (int)process[0];
    process[0]->tss.ss0 = KERNEL_DS;
    process[0]->tss.cr3 = sys_pgdir;
    process[0]->tss.es = TASK_DS;
    process[0]->tss.cs = TASK_CS;
    process[0]->tss.ss = TASK_DS;
    process[0]->tss.ds = TASK_DS;
    process[0]->tss.fs = TASK_DS;
    process[0]->tss.gs = TASK_DS;
    process[0]->tss.ldt_selector = TASK_LDT;
    process[0]->tss.io_base = sizeof(struct_tss);
    /*启用时钟中断*/
    set_irq_handler(IRQ_CLOCK,irq_timer_handler);
    enable_irq(IRQ_CLOCK);
}
