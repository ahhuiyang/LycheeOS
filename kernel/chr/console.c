/***************************************************************************
 *
 *  console.c-显示器控制
  *     常见的显示器模式有MDA,CGA,EGA,VGA
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
#include "io.h"
#include "const.h"
#include "type.h"
#include "console.h"
#include "string.h"
#include "oslib.h"
#include "tty.h"
 
 /*
  *这些参数要在启动时配置好，放在固定位置，由rmtopm.bin写入
  */
/*光标位置*/  
#define ORI_VIDEO_CURSOR_X  (*(u16 *)0x9000C)
#define ORI_VIDEO_CURSOR_Y  (*(u16 *)0x9000E)
/*显示模式*/
#define ORI_VIDEO_MODE      (*(u16 *)0x9000A)
/*显示行数*/
#define ORI_VIDEO_COLS      (*(u16 *)0x90006)
/*显示列数*/
#define ORI_VIDEO_ROWS      (*(u16 *)0x90008)
/*int 10中断,ah=0x12,bl=0x10时的AX,BX*/
#define ORI_VIDEO_AX        (*(u16 *)0x90002)
#define ORI_VIDEO_BX        (*(u16 *)0x90004)

/*pre declaration*/
public void set_console_origin(struct_console *console);

/*这些局部变量用于标识显示器操作位置和参数*/
private u8 video_type;                  /*显示器类型，MDA、EGAM、EGA/VGA、CGA*/
private u8 video_rows_per_page;         /*一屏幕字符行数*/
private u8 video_cols_per_page;         /*一屏幕字符列数*/
private u8 video_char_size;             /*每个字符在显存中占用字节数*/
private unsigned int video_row_size;    /*每行占用字节数*/
private unsigned int video_mem_start;   /*显存起始地址*/
private unsigned int video_mem_size;    /*显存大小*/
private u8 video_ctl_reg;               /*显示器控制寄存器*/
private u8 video_data_reg;              /*数据寄存器*/
private u16 video_erase_char;  /*使用什么字符作为擦除字符*/
private u8 video_char_attr = 0x07;      /*黑底白字*/

/*defined in tty.c*/
extern unsigned int nr_current_tty;

/*设置当前的显示器*/
public void set_current_console(struct_tty *current_tty)
{
    set_console_origin(current_tty->console);
}

/*设置显存原始位置，这在屏幕滚动时用*/
public void set_console_origin(struct_console *console)
{
    unsigned int address = 
        console->start_mem_addr + console->top * video_row_size;
        
    disable_int();
    
    /*设置显存起始地址*/
    out_byte(video_ctl_reg,VIDEO_START_ADDR_H);
    out_byte(video_data_reg,(address >> 8) & 0xFF);
    out_byte(video_ctl_reg,VIDEO_START_ADDR_L);
    out_byte(video_data_reg,(address) & 0xFF);
    
    enable_int();
}

/*移动光标*/
private void set_console_cursor(struct_console *console)
{
    int pos = console->cursor_pos;
    /*此过程不可中断*/
    disable_int();
    
    out_byte(video_ctl_reg,VIDEO_CURSOR_H);
    out_byte(video_data_reg,((pos/2)>>8) & 0xFF);
    out_byte(video_ctl_reg,VIDEO_CURSOR_L);
    out_byte(video_data_reg,((pos/2) & 0xFF));
    
    enable_int();
}

private inline unsigned int getx(unsigned int base,unsigned int cursor_pos)
{
    return (cursor_pos) % (video_row_size) / 2;
}

private inline unsigned int gety(unsigned int base,unsigned int cursor_pos)
{
    return (cursor_pos - base) / (video_row_size);
}

/*将光标移动到指定位置*/
private void gotoxy(struct_console *console,unsigned int x,unsigned int y)
{
    int i;
    unsigned int start_mem = console->start_mem_addr;
    unsigned int mem = start_mem 
        + x * video_row_size + y * video_char_size;
        
    if(mem > console->current_mem_addr)return;
        
        /*如果移动到的显示内存未超过位置，则直接移动*/
    if(mem <= start_mem + console->mem_size)
    {
        console->cursor_pos = mem;
        if(mem > console->current_mem_addr){
            console->current_mem_addr = mem;
        }
        
             /*设置光标*/
            set_console_cursor(console);
            /*如果当前光标处于屏幕之外，则将其移回屏幕中，滚动*/
            if(x > console->top + video_rows_per_page)
            {
                for(i = 0 ; 
                    i < (x - (console,console->top + video_rows_per_page)); 
                    i++)
                {
                    /*top会在 scroll_up函数里设置，屏幕窗口向下移动*/
                    scroll_up(console);
                }
            }
            else if(x < console->top)
            {
                for(i = 0 ; i < (console->top - x + 1) ; i ++)
                {
                    scroll_down(console);
                }
            }
    }
}

/*将显示屏幕窗口向下滚动一行，如果超出显存位置，则自动将下面的数据移到初始位置*/
public void scroll_up(struct_console *console)
{
    console->top ++;
    
    if(console->top > 
        (console->mem_size / video_row_size - video_rows_per_page))
    {
        memcpy((char *)console->start_mem_addr,
            (char *)console->start_mem_addr + video_row_size,
            console->mem_size-video_row_size);
        memset((char *)console->start_mem_addr + console->mem_size - video_row_size,
            0,video_row_size);
    }
    
    set_console_origin(console);
}

/*将屏幕窗口向下滚动指定的行数，并更新console.top*/
public void scroll_down(struct_console *console)
{
    console->top --;
    if(console->top < 0)console->top = 0;
    set_console_origin(console);
}

/*将显示内存上移一行*/
private void video_mem_move_up(struct_console *console)
{
    memcpy((char *)console->start_mem_addr,
        (char *)console->start_mem_addr + video_row_size,
        console->mem_size - video_row_size);
    /*将新行内存全部清零*/
    memset((char *)console->start_mem_addr + (console->mem_size - video_row_size),
        0,video_row_size);
        
        /*保持原来的光标位置和当前字符位置不变*/
        console->current_mem_addr -= video_row_size;
        console->cursor_pos -= video_row_size;
        /*顶行号减一*/
        console->top --;
        
        set_console_cursor(console);
}

/*换行，将光标下移一行*/
private void lf(struct_console *console)
{
    /*如果光标不在倒数第一行*/
    if(gety(console->cursor_pos,1) < (console->top + video_rows_per_page))
    {
        console->cursor_pos += video_row_size;
        /*设置字符到达的位置*/
        if(console->cursor_pos > console->current_mem_addr)
            console->current_mem_addr = console->cursor_pos;
            
        set_console_cursor(console);
        return;
    }
    else    /*否则，将屏幕内容上移一行，注意会移动显存数据*/
    {
        scroll_up(console);
    }
}

/*换行，将光标上移一行，reverse line feed*/
private void rlf(struct_console *console)
{
    /*如果光标不在最顶行*/
    if(gety(console->start_mem_addr,console->cursor_pos) > console->top)
    {
        console->cursor_pos -= video_row_size;
        set_console_cursor(console);
        return;
    }
    else
    {
        scroll_down(console);
    }
}

/*回车，将光标移到当前行的最前面来*/
private void cr(struct_console *console)
{
    console->cursor_pos = console->cursor_pos 
        - getx(console->start_mem_addr,console->cursor_pos) * video_char_size;
    set_console_cursor(console);
}

/*删除光标前一字符，用清除符代替*/
private void del(struct_console *console)
{
    /*只能删除当前行，不能折回到上行*/
    if(getx(console->start_mem_addr,console->cursor_pos))
    {
        console->cursor_pos -=2;
        *(u16 *)console->cursor_pos = video_erase_char;
        set_console_cursor(console);
    }
}

/*强制删除，如果当前处于0列，删除上一行最后一个字符，用空格代替*/
private void del_lf(struct_console *console)
{
    /*只要不在初始位置，就可删除*/
    if(console->cursor_pos >= console->start_mem_addr + 2)
    {
        console->cursor_pos -= 2;
        *(u16 *)console->cursor_pos = video_erase_char;
        set_console_cursor(console);
    }
}

/*在光标位置插入一个字符，字符插入后，所有其它字符向后移动*/
private void insert_char(struct_console *console,unsigned char c)
{
    /*只处理可打印字符*/
    if(c >= 0x20 && c <0x80)
    {
        if(console->current_mem_addr + 2 >= 
            console->start_mem_addr + console->mem_size)
            {
                /*将当前显存中所有内容上移一行，以此璔加新行*/
                video_mem_move_up(console);
            }
            
            memcpy((char *)console->cursor_pos,(char *)console->cursor_pos + 2,
                console->current_mem_addr - console->cursor_pos);
                
            console->current_mem_addr += 2;
            
            /*写新字符*/
            *(unsigned char*)(console->cursor_pos ++) = c;
            *(unsigned char*)(console->cursor_pos ++) = video_char_attr;
            
            /*设置新的光标位置*/
            set_console_cursor(console);    
    }
}

/*插入一行*/
private void insert_line(struct_console *console)
{
    /*如果当前字符已在最后一行了，则增加新一行*/
    if(gety(console->start_mem_addr,console->current_mem_addr) >= 
        console->mem_size / video_row_size - 1)
        {
            video_mem_move_up(console);
            video_mem_move_up(console);
        }
        
        /*字符串向后拷贝*/
    memcpy((char *)console->cursor_pos,
        (char *)((gety(console->start_mem_addr,console->cursor_pos) + 2) * video_row_size),
        console->current_mem_addr - console->cursor_pos);
            
    memset((char *)console->cursor_pos,0,
        (gety(console->start_mem_addr,console->cursor_pos) + 2) * video_row_size - video_char_size);
            
    console->current_mem_addr += video_row_size;
    
    /*光标出现在新的空行上*/
    console->cursor_pos = (gety(console->start_mem_addr,console->cursor_pos) + 1) * video_row_size;
                 
    set_console_cursor(console);
}

/*删除光标处的一个字符*/
private void delete_char(struct_console *console)
{
    if(console->cursor_pos >= console->start_mem_addr + 2)
    {
        console->cursor_pos -=2;
        memcpy((char *)console->cursor_pos,(char *)console->cursor_pos + 2,
            console->current_mem_addr - console->cursor_pos - 2);
        console->current_mem_addr -= 2;
        memset((char *)console->current_mem_addr,0,2); 
    }
}

/*删除光标处的一行*/
private void delete_line(struct_console *console)
{
    memset((char *)(gety(console->start_mem_addr,console->cursor_pos) * video_row_size),0,video_row_size);
}

/*最重要的函数，显示器写*/
public void console_write(struct_tty *tty)
{
    unsigned char c;
    int i;
    /* 如果还有字符，就一直循环*/
    while(c = (get_char_from_write(tty) & 0xFF))
    {
        switch(c)
        {
            case '\t':
                for(i = 0 ; i <= 7 ; i ++)
                {/*insert 8 space*/
                    insert_char(tty->console,0x20);      /*插入八个空格*/
                }/*end for*/
                break;
            case '\b':
                delete_char(tty->console);
                break;
            case 'n':
                cr(tty->console);
                lf(tty->console);
                break;
            default:
                insert_char(tty->console,c);
                break;
        }
    }
}

/*显示器初始化,获得所有参数*/
public void console_init()
{
    int i;
    
    video_rows_per_page = ORI_VIDEO_ROWS;
    video_cols_per_page = ORI_VIDEO_COLS;
    video_char_size = 2;
    video_row_size = video_char_size * video_cols_per_page;
    video_erase_char = 0x0720;          /*白色空格*/
    
    /*下面判断显示器参数*/
    if(ORI_VIDEO_MODE == 0x07)
    {
        video_mem_start = (u32)MONO_MEM_START;
        video_ctl_reg = (u8)MONO_ADDR_REG;
        video_data_reg = (u8)MONO_DATA_REG;
        /*下面判断显存大小,依据为是否支持int10 ah=0x12,bl = 0x10,支持 则为MGA*/
        if(ORI_VIDEO_BX & 0x0ff != 0x10)
        {
            video_mem_size = MONO_EGA_MEM_SIZE;
            video_type = VIDEO_EGAM;
        }
        else
        {
            video_mem_size = MONO_MDA_MEM_SIZE;
            video_type = VIDEO_MDA;
        }
    }
    else
    {
        video_mem_start = (u32)CRT_MEM_START;
        video_ctl_reg = (u8)CRT_ADDR_REG;
        video_data_reg = (u8)CRT_DATA_REG;
        
        if(ORI_VIDEO_BX & 0x0ff != 0x10)
        {
            video_mem_size = CRT_VGA_MEM_SIZE;
            video_type = VIDEO_VGA;
        }
        else
        {
            video_mem_size = CRT_CGA_MEM_SIZE;
            video_type = VIDEO_CGA;
        }
    }
    
    /*初始化控制台参数*/
    for(i = 0 ; i <= NR_CONSOLE ; i ++)
    {
        console_table[i].start_mem_addr = 
            video_mem_start + i * (video_mem_size / NR_CONSOLE);
        console_table[i].current_mem_addr = 
            console_table[i].start_mem_addr;
        console_table[i].mem_size = video_mem_size / NR_CONSOLE;
        console_table[i].cursor_pos = console_table[i].start_mem_addr;
        console_table[i].top = 0;
    }
}
