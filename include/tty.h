/***************************************************************************
 *
 *  tty.h-tty常量和结构的定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "console.h"
 
#ifndef _TTY_H
#define _TTY_H

#define TTY_BUFFER_SIZE 1024
#define NR_TTY          4

/*控制标志*/
#define TTL_CTL_ECHO    0x00000001

typedef struct _struct_tty_buffer
{
    unsigned int *p_head;
    unsigned int *p_tail;
    unsigned int count;
    unsigned int buffer[TTY_BUFFER_SIZE];
}struct_tty_buffer;
 
typedef struct _struct_tty
{
    unsigned int output_flags;
    unsigned int input_flags;
    unsigned int ctl_flags;
    /*一个进程对应一个终端，如果当前终端未使用,pid=0*/
    unsigned int pid;
    /*终端写函数*/
    int (*write)(unsigned int nr_tty,char *buffer,int len);
    /*读取队列*/
    struct_tty_buffer read_buffer;
    struct_tty_buffer write_buffer;
    /*一个终端跟一个显示器有关*/
    struct_console *console;
}struct_tty;

/*现在定义8个tty*/
public extern struct_tty tty_table[NR_TTY];

/*这些函数在tty.c中实现，其中有部分现在还未实现*/
public void put_key_to_tty_read(struct_tty *tty,u32 key);
public void put_key_to_tty_write(struct_tty *tty,unsigned int key);
/*从tty的write_buffer里读取可显示的字符及部分控制字符*/
public unsigned char get_char_from_write(struct_tty *tty);
public unsigned int is_current_tty(struct_tty *tty);
public int tty_read(unsigned int nr_tty,char *buffer,int len);
public int tty_write(unsigned int nr_tty,char *buffer,int len);
public void task_tty(void);
public struct_tty *get_empty_tty(unsigned int pid);
public void release_tty(struct_tty *tty);
public void set_current_tty(unsigned int nr);
public void init_tty();

#endif
