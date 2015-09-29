/***************************************************************************
 *
 *  syscall.h-系统调用相关原型、常量和数组
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _SYSCALL_H
#define _SYSCALL_H

/*声明为void*，这样就可以加入任何类型的函数了，编译器不会报错和警告*/
typedef void* fn_sys_call;

#define NR_SYS_CALLS        5
#define NR_INT_SYS_CALL     0x80

/*defined in syscall.asm*/
public extern int sys_fork(void);
public extern int sys_pause(void);
public extern int sys_write(u32 fhandle,char *buffer,int count);
public extern int sys_read(u32 fhandle,char *buffer,int count);
public extern int sys_getpid();

/*系统调用函数表*/
public extern fn_sys_call sys_call_table[NR_SYS_CALLS];

#endif
