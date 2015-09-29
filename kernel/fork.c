/***************************************************************************
 *
 *  fork.c-创建进程的辅助函数
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "kernel.h"
#include "memory.h"
#include "type.h"
#include "process.h"
 
public int sys_last_pid = 0;
 
 /*
 *copy_memory
 *将当前进程的页表和页目录复制到新进程
 */
public boolean copy_memory(int proc_nr,struct_process *proc)
{
    u32 old_code_base,old_data_base,code_limit,data_limit;
    u32 new_code_base,new_data_base;
    
    /*下面主要是检测代码段和数据段长度和限长是否相同，如果不同，则出错*/
    code_limit = get_seg_limit(&current->ldt[INDEX_USER_CS]);
    data_limit = get_seg_limit(&current->ldt[INDEX_USER_DS]);
    if(code_limit != data_limit)
    {
        halts("unequal seg limit.\n");
    }
    
    old_code_base = get_seg_base(current->ldt[INDEX_USER_CS]);
    old_data_base = get_seg_base(current->ldt[INDEX_USER_DS]);
    if(old_code_base != old_data_base)
    {
        halts("unequal seg base.\n");
    }
    
    /*新进程默认的代码和数据基址是其进程号乘以0x4000000*/
    new_code_base = new_data_base = proc_nr * PROCESS_MEM_SPACE;
    proc->start_code = new_code_base;
    set_seg_base(&current->ldt[INDEX_USER_CS],new_code_base);
    set_seg_base(&current->ldt[INDEX_USER_DS],new_data_base);
    
    /*把当前进程的页目录和页表复制到p指向的进程页目录和页表位置*/
    if(copy_page_tables(old_code_base,new_code_base,data_limit))
    {
        free_page_tables(new_code_base,data_limit);
        return FALSE;
    }
    
    /*执行到这里说明函数执行成功了*/
    return TRUE;
}

/*通过递增，取得下一个进程号*/
public int get_last_pid()
{
    return (sys_last_pid ++);
}

/*
*将当前进程结构体复制到指定的进程结构中
*该函数返回新进程的pid，返回0表示失败
*/
public int copy_process(int bad_eip,int proc_nr,int ebx,int ecx,int edx,int gs,
    int fs,int es,int ds,int ebp,int edi,int esi,int eip,int cs,
    int eflags,int esp,int ss)
{
    struct_process *proc;
    struct_file *file;
    int i;
    
    /*为任务结构分配一页物理内存，部分用于进程的内核堆栈*/
    if(!(proc = (struct_process *)get_phy_page()))
    {
        return 0;
    }
    
    /*将地址赋给进程数组*/
    process[proc_nr] = proc;
    
    /*首先，把当前进程的进程体结构内容拷贝给新进程，然后有针对地更改*/
    *proc = *current;
    
    /*下面初始化这个进程*/
    proc->state = PROCESS_WAIT_UNINTERRUPTIBLE;
    proc->pid = get_last_pid();
    proc->ppid = current->pid;
    proc->user_time = 0;
    proc->kernel_time = 0;
    proc->create_time = CURRENT_TIME;
    proc->tty = &tty_table[nr_current_tty];
    proc->tss.pre_tss_selector = 0;
    proc->tss.reserved1 = 0;
    proc->tss.esp0 = PAGE_SIZE + (u32)proc;
    proc->tss.ss0 = KERNEL_DS;
    proc->tss.eip = eip;
    proc->tss.eflags = eflags;
    proc->tss.eax = 0;
    proc->tss.ecx = ecx;
    proc->tss.edx = edx;
    proc->tss.ebx = ebx;
    proc->tss.esp = esp;
    proc->tss.ebp = ebp;
    proc->tss.esi = esi;
    proc->tss.edi = edi;
    proc->tss.es = es & 0xFFFF;
    proc->tss.cs = cs & 0xFFFF;
    proc->tss.ss = ss & 0xFFFF;
    proc->tss.ds = ds & 0xFFFF;
    proc->tss.fs = fs & 0xFFFF;
    proc->tss.gs = fs & 0xFFFF;
    proc->tss.ldt_selector = TASK_LDT;
    proc->tss.tflag = 0;
    proc->tss.io_base = sizeof(struct_tss);
    
    /*复制页表*/
    if(!copy_memory(proc_nr,proc))
    {
        /*复制不成功*/
        process[proc_nr] = NULL;
        free_phy_page((u32)proc);
        return 0;
    }
    
    /*
    *如果父进程有文件是打开的，则将对应文件的打开次数增1
    *因为这里创建的子进程会与父进程共享这些打开的文件
    *同样，也就当前进程的pwd,root,image的引用次数均增1
    */
    for(i = 0 ; i < NR_MAX_FILES ; i ++)
    {
        if(proc->files[i])
        {
            proc->files[i]->count ++;
        }
    }
    /*工作目录*/
    if(proc->pwd)
    {
        proc->pwd->count ++;
    }
    /*系统根*/
    if(proc->root)
    {
        proc->root->count ++;
    }
    /*可执行文件*/
    if(proc->image)
    {
        proc->image->count ++;
    }
    
    /*
    *让新进程处于就绪态，这样CPU就可能会调度此程序
    *新进程运行起始位置处在进入此系统调用之前，父进程的运行位置
    */
    proc->state = PROCESS_RUNNING;
    
    /*返回新进程的进程号*/
    return proc->pid;
}

/*
*从进程数组里找到一个未用的项，如果找到了，返回找到的位置索引
*否则返回0，返回0表示失败
*/
public int get_free_process()
{
    int i;
    
    /* 排除进程0*/
    for(i = 1 ; i < NR_PROCESS ; i ++)
    {
        if(!process[i])
        {
            return i;
        }
    }
    
    return 0;
}
