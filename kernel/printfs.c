/***************************************************************************
 *
 *  sysprintf.c-主要实现sysprintf函数
 *  用于可变参数的打印
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "stdarg.h"
#include "kernel.h"
#include "string.h"
#include "tty.h"

char buffer[TTY_BUFFER_SIZE];       /*1024bytes*/

/*此函数只能供系统调用*/
int printfs(const char *fmt,...)
{
    va_list args;
    int i;
    
    va_start(args,fmt);
    i = vsprintf(buffer,fmt,args);
    
    tty_write(nr_current_tty,buffer,i);
    
    return i;
}
