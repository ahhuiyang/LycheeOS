/***************************************************************************
 *
 *  string.h-字符串和内存操作函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _STRING_H
#define _STRING_H

#include "type.h"
#include "stdarg.h"

#define CHAR_SPACE  ' '

public char* memcpy(char* dst,char* src,unsigned int count);
public void* memset(void* s,char c,unsigned int count);
public unsigned int strlen(char *str);
#define strncpy(dst,src,len) (memcpy((char *)dst,(char *)src,len))
public unsigned int strchr(char *str,char c,int len);
public int set_bit(void *addr,unsigned int nr);
public int clear_bit(void *addr,unsigned int nr);               /*清除一个逻辑块的数据*/
#define clear_block(addr) (memset(addr,0,BLOCK_SIZE))
public int find_first_zero(void *addr);

/*下面的函数定义在vsprintf.c中*/
/*将格式字符串转换成相应的结果字符串*/
public int vsprintf(char *buf,const char *fmt,va_list args);

/*系统打印函数*/
int printfs(const char *fmt,...);

/*从CMOS中读取一字节的数据*/
public u8 read_cmos(u8 addr);

#endif
