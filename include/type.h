/***************************************************************************
 *
 *  type.h-定义数据类型及相关操作
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _TYPE_H
#define _TYPE_H

/*函数或变量的作用域*/
#define public
#define private static

/*传入或传出参数*/
#define in
#define out

typedef int boolean;

/*无符号指定长度的数据类型*/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef struct _struct_u64
{
    unsigned int a;
    unsigned int b;
}u64;
typedef struct _struct_u128
{
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
}u128;

/*用于硬盘数据的，主要是保证要有32字节长*/
typedef struct _struct_hd_info
{
    char data[32];
}struct_hd_info;

typedef char *cstr;

/*中断处理程序接口函数原型*/
typedef void (*int_handler)();

#define NULL ((void *)0)
#define null NULL

#define TRUE    1
#define FALSE   0
#define true    TRUE
#define false   FALSE

#endif
