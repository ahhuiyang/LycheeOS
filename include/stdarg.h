/***************************************************************************
 *
 *  stdarg.h-可变参数函数调用的类型声明
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _STDARG_H
#define _STDARG_H

#include "type.h"

typedef char *va_list;

/*
*有些类型，如char,short,double等，不足4字节或超过4字节
*这些类 型在堆栈上要么占4字节，要么占整倍的4字节
*/
#define _va_rounded_size(TYPE)\
    (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

/*
*对于printf来说，第一个参数是字符串，第一个参数中的参数放在第二个参数及以后
*va_start让ARGS指向真正的参数，即第二个参数
*其指向的是堆栈位置
*/
#define va_start(ARGS,FMT)\
    (ARGS = ((char *)&(FMT) + _va_rounded_size(FMT)))

/*
*取当前参数指针，并把指针移向下一参数
*返回的是堆栈中存放的值，如果是字符串，则返回指向字符串的指针
*/
#define va_arg(ARGS,TYPE)\
    (ARGS += _va_rounded_size(TYPE),\
    *((TYPE *)(ARGS - _va_rounded_size(TYPE))))
    
/*将ARGS指向空位置*/
#define va_end(ARGS)\
    (ARGS = NULL)

#endif
