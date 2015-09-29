/***************************************************************************
 *
 *  syscall.c-一些系统调用的实现及相关操作函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 #include "type.h"
 #include "syscall.h"
 #include "kernel.h"
 
public fn_sys_call sys_call_table[NR_SYS_CALLS] = {
    sys_fork,sys_pause,
    sys_write,sys_read,sys_getpid
};
 
 /*系统调用最外层入口，在syscall.asm中定义和实现*/
void sys_call(void);
 
public void set_system_call(unsigned int nr_call,void *fn_sys_call)
{
    if(nr_call >=0 && nr_call <= NR_SYS_CALLS)
    {
        sys_call_table[nr_call] = fn_sys_call;
    }
}
 
public void syscall_init()
{
    set_system_gate(NR_INT_SYS_CALL,sys_call);
}
