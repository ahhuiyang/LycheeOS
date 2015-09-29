/***************************************************************************
 *
 *  tty.c-终端，综合读写
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "type.h"
#include "tty.h"
#include "console.h"
#include "keyboard.h"
#include "io.h"
#include "oslib.h"

/*pre declaration*/
public void put_char_to_tty_write(struct_tty *tty,char c);
 
public unsigned int nr_current_tty;
public extern struct_tty tty_table[NR_TTY];
 
/*将key放入读取队列*/
public void put_key_to_tty_read(struct_tty *tty,u32 key)
{
    if(tty->read_buffer.count < TTY_BUFFER_SIZE)
    {
        *(tty->read_buffer.p_head ++) = key;
        if(tty->read_buffer.p_head 
            == tty->read_buffer.buffer + TTY_BUFFER_SIZE)
        {
            tty->read_buffer.p_head = tty->read_buffer.buffer;
        }
        tty->read_buffer.count ++;
    }
    
    /*如果带有回显功能，则将键放入写队列*/
    if(tty->ctl_flags & TTL_CTL_ECHO)
    {
        /*所有键放入队列，显示驱动决定如何显示*/
        if(!(key & KEYMASK_EXT))
        {
        put_char_to_tty_write(tty,key);
        }
    }
}

public void put_key_to_tty_write(struct_tty *tty,unsigned int key)
{
    if(tty->write_buffer.count < TTY_BUFFER_SIZE)
    {
        *(tty->write_buffer.p_head ++) = key;
        if(tty->write_buffer.p_head 
            == tty->write_buffer.buffer + TTY_BUFFER_SIZE)
        {
            tty->write_buffer.p_head = tty->write_buffer.buffer;
        }
        tty->write_buffer.count ++;
    }
}

public void put_char_to_tty_write(struct_tty *tty,char c)
{
    u32 key = 0 | c;
    
    put_key_to_tty_write(tty,key);
}

public u32 get_key_from_tty_read(struct_tty *tty)
{
    u32 key = 0;
    
    if(tty->read_buffer.count > 0)
    {
        key = *(tty->read_buffer.p_tail ++);
        if(tty->read_buffer.p_tail == tty->read_buffer.buffer + TTY_BUFFER_SIZE)
        {
            tty->read_buffer.p_tail = tty->read_buffer.buffer;
        }
        tty->read_buffer.count --;
    }
    
    return key;
}

public unsigned char get_char_from_write(struct_tty *tty)
{
    unsigned int key = 0;
    unsigned char cRet = 0;
    unsigned int crlf = 0;
    
    if(tty->write_buffer.count > 0)
    {
        key = *(tty->write_buffer.p_tail ++);
        if(tty->write_buffer.p_tail == tty->write_buffer.buffer + TTY_BUFFER_SIZE)
        {
            tty->write_buffer.p_tail = tty->write_buffer.buffer;
        }
        tty->write_buffer.count --;
    }
    
    /*现在我们得到了键，下面开始分析，可打印字符直接传送*/
    if(!(key & KEYMASK_EXT))
    {
        cRet = key & KEYMASK_PRINT_CHAR;
    }
    else
    {
        switch(key & 0xFF)
        {
            case ENTER:
                cRet = '\n';
                break;
            case TAB:
                cRet = '\t';
            case BACKSPACE:
                cRet = '\b';
        }
    }
    
    return cRet;
}

public unsigned int is_current_tty(struct_tty *tty)
{
    return (&tty_table[nr_current_tty] == tty);
}

/*
*
*
*从tty里对外的读写接口
*通过系统调用向外界提供标准输入输出的功能
*
*/
public int tty_read(unsigned int nr_tty,char *buffer,int len)
{
    int nr_read = 0;
    u32 key = 0;
    int i = 0;
    
    if(!len)return 0;
    
    while(nr_read < len)
    {
        while(!(key = get_key_from_tty_read(&tty_table[nr_tty])));
        
        buffer[nr_read] = (char)((key & KEYMASK_PRINT_CHAR) & 0xFF);
        
        nr_read ++;
    }
    
    return nr_read;
}

public int tty_write(unsigned int nr_tty,char *buffer,int len)
{
    int i;
    
    for(i = 0 ; i < len ; i ++)
    {
        put_char_to_tty_write(&tty_table[nr_tty],buffer[i]);
    }
    
    console_write(&tty_table[nr_tty]);
    
    return len;
}

/*tty任务调用函数，必须不断执行*/
public void task_tty(void)
{
    while(1)
    {
        keyboard_read(&tty_table[nr_current_tty]);
        console_write(&tty_table[nr_current_tty]);
    }
}

/*查找可用的tty*/
public struct_tty *get_empty_tty(unsigned int pid)
{
    unsigned int i;
    
    for(i = 0 ; i <= NR_TTY ; i ++)
    {
        if(tty_table[i].pid == 0)
        {
            break;
        }
    }
    
    /*没找到，返回空值*/
    if(i == NR_TTY + 1)return null;
    
    /*找到了，初始化清零*/
    tty_table[i].read_buffer.p_head = tty_table[i].read_buffer.buffer;
    tty_table[i].read_buffer.count = 0;
    tty_table[i].write_buffer.p_head = tty_table[i].write_buffer.buffer;
    tty_table[i].write_buffer.count = 0;
    
    tty_table[i].console->current_mem_addr = tty_table[i].console->start_mem_addr;
    tty_table[i].console->cursor_pos = tty_table[i].console->start_mem_addr;
    tty_table[i].pid = pid;
    
    return &tty_table[i];
}

/*放弃tty*/
public void release_tty(struct_tty *tty)
{
    tty->pid = 0;
}

/*设置当前的tty*/
public void set_current_tty(unsigned int nr)
{
    if(nr >= 0 && nr <= NR_TTY && nr_current_tty != nr)
    {
        nr_current_tty = nr;
        
         /*下面要设置显示器了，这段代码不能被中断*/
        set_current_console(nr);
    }
}

/*从键盘读取的*/
public void sys_key_process(struct_tty *tty,u32 key)
{   
    /*表明此键是否正在使用*/
    unsigned int used = 0;
    
    if(((key & KEYMASK_ALT_L) || (key & KEYMASK_ALT_R))
        && ((key & KEYMASK_KEY) >= F1 && (key & KEYMASK_KEY) <= F4))
        {
            set_current_console((key & KEYMASK_KEY) - F1);
            used = 1;
        }
        
    /*Shift + 向上箭头和向下箭头，滚屏*/
    if(((key & KEYMASK_SHIFT_L) || (key & KEYMASK_SHIFT_R))
        && ((key & KEYMASK_KEY == UPARROW) || (key & KEYMASK_KEY == NUMPAD_UP)))
        {
            scroll_down(tty_table[nr_current_tty].console);
            used = 1;
        }
        
        if(((key & KEYMASK_SHIFT_L) || (key & KEYMASK_SHIFT_R))
        && ((key & KEYMASK_KEY == DOWNARROW) || (key & KEYMASK_KEY == NUMPAD_DOWN)))
        {
            scroll_up(tty_table[nr_current_tty].console);
            used = 1;
        }
        
    if(!used)
    {
        put_key_to_tty_read(tty,key);
    }
}

public void init_tty()
{   
    int i;
    
    /*初始化键盘*/
    init_keyboard();
    /*初始化显示器*/
    init_console();
    
    for(i = 0 ; i < NR_TTY ; i ++)
    {
        tty_table[i].output_flags = 0;
        tty_table[i].input_flags = 0;
        /*带回显*/
        tty_table[i].ctl_flags = TTL_CTL_ECHO;
        /*默认没有使用*/
        tty_table[i].pid = 0;
        tty_table[i].write = tty_write;
        tty_table[i].console = &console_table[i];
    }
}
