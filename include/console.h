/***************************************************************************
 *
 *  console.h-显示器常量和结构的定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "type.h"

#define NR_CONSOLE  4

typedef struct _struct_console
{
    unsigned int start_mem_addr;    /*物理位置*/
    unsigned int current_mem_addr;  /*物理位置*/
    unsigned int mem_size;          /*空间大小*/
    unsigned int cursor_pos;        /*光标位置，物理*/
    /*滚动屏幕，顶行*/
    unsigned int top;               /*屏幕顶端行，相对起始位置*/
}struct_console;

public struct_console console_table[NR_CONSOLE];

/*函数原型，定义在console.c中*/
/*将显示屏幕窗口向下滚动一行，如果超出显存位置，则自动将下面的数据移到初始位置*/
public void scroll_up(struct_console *console);
/*将屏幕窗口向上滚动指定的行数，并更新console.top*/
public void scroll_down(struct_console *console);

#endif
