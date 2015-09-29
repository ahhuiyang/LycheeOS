/***************************************************************************
 *
 *  stdcall.h-系统调用接口
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _STDCALL_H
#define _STDCALL_H
 
/*系统调用号*/
#define NR_fork     0
#define NR_pause    1
#define NR_write    2
#define NR_read     3
#define NR_getpid   4

/*下面的宏传入具体的值就会形成一个系统调用的声明和实现*/
/*第一个，对于没有参数的系统调用*/
#define syscall0(type,name) \
type name(void) \
{ \
    int ret; \
    __asm__ volatile ("int $0x80":"=a"(ret):"0"(NR_##name)); \
    return ret;\
}

/*第二个，只有一个参数的系统调用*/
#define syscall1(type,name,a_type,a) \
type name(a_type a) \
{ \
    int ret; \
    __asm__ volatile("int $0x80":"=a"(ret):"0"(NR_##name),"b"((int)(a))); \
    return ret; \
}

/*第三个，有两个参数的系统调用*/
#define syscall2(type,name,a_type,a,b_type,b) \
type name(a_type a,b_type b) \
{ \
    int ret; \
    __asm__ ("int $0x80":"=a"(ret):"0"(NR_##name),"b"((int)(a)),"c"((int)(b))); \
    return ret; \
}

/*第四个，有三个参数的系统调用*/
#define syscall3(type,name,a_type,a,b_type,b,c_type,c) \
type name(a_type a,b_type b,c_type,c) \
{ \
    int ret; \
    __asm__ ("int $0x80":"=a"(ret):"0"(NR_##name),"b"((int)(a)),"c"((int)(b)),"d"((int)(c))); \
    return ret; \
}

#endif
