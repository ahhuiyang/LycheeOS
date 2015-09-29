/***************************************************************************
 *
 *  keyboard.c-键盘操作函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "keyboard.h"
#include "keymap.h"
#include "tty.h"
#include "type.h"
#include "oslib.h" 
#include "interrupt.h"
 
/*
 *将函数声明为public，这样外部模块可以调用
 *
 *public和private的定义如下：
 * #define public
 * #define private static
 *
 *public只起修饰作用，表明这是全局变量，其它模块可调用
 *private只当在当前文件作用局内使用
 */
  
private u8 caps_lock;
private u8 num_lock;
private u8 scroll_lock;

private u8 flag_shift_l,flag_shift_r;
private u8 flag_ctrl_l,flag_ctrl_r;
private u8 flag_alt_l,flag_alt_r;

/*键盘读入的原始数据缓冲*/
private struct_keyboard kbuffer;

/*pre declaration*/
private void set_leds();
  
/*键盘中断处理程序原型，当一个键按下或弹起时，该函数会被调用*/
public void irq_keyboard_handler()
{
    /*读入扫描码*/
    u8 scan_code = in_byte(KEYBOARD_DATA);
    /*加入缓冲区，如果缓冲区不足，丢弃*/
    
    if(kbuffer.count < KEYBOARD_BUFFER_SIZE)
    {
        *(kbuffer.p_head) == scan_code;
        kbuffer.p_head ++;
        if(kbuffer.p_head == kbuffer.buffer + KEYBOARD_BUFFER_SIZE)
        {
            kbuffer.p_head = kbuffer.buffer;
        }
        
        kbuffer.count++;
    }
}

/*键盘初始化函数，在main函数里调用*/
public void init_keyboard()
{
    flag_shift_l = 0;
    flag_shift_r = 0;
    flag_ctrl_l = 0;
    flag_ctrl_r = 0;
    flag_alt_l = 0;
    flag_alt_r = 0;
    
    kbuffer.p_head = kbuffer.buffer;
    kbuffer.p_tail = kbuffer.buffer;
    kbuffer.count = 0;
    
    set_leds();
    
    /*安装键盘中断*/
    set_irq_handler(IRQ_KEYBOARD,irq_keyboard_handler);
    /*打开键盘中断*/
    enable_irq(IRQ_KEYBOARD);
}

/*
*   向键盘的8042的数据缓冲中写数据时，必须等缓冲为空，通过读状态端口0x64
*/
private void keyboard_wait()
{
    unsigned char state;
    
    do
    {
        state = in_byte(KEYBOARD_CTL);
    }while(state & 0x02);
}

/*
*   往8048发送命令，命名用端口0x60，设置LED的命令是0xED
*   当键盘收到这个命令后，会回复一个确认(0xFA，在端口0x60)
*   必须等待这个确认
*/
private void keyboard_wait_ack()
{
    int state;
    
    do
    {
        state = in_byte(KEYBOARD_DATA);
    }while(state != 0xFA);
}

/*设置键盘灯*/
private void set_leds()
{
    u8 leds = (caps_lock << 2) | (num_lock << 1) | (scroll_lock);
    
    /*等待输入缓冲区为空*/
    keyboard_wait();
    /*请求设置led灯*/
    out(KEYBOARD_DATA,0xED);
    /*等待确认*/
    keyboard_wait_ack();
    /*已确认，向0x60发送设置led灯的字节*/
    out_byte(KEYBOARD_DATA,leds);
}

private unsigned char read_byte_form_keyboard()
{
    unsigned char scan_code;
    
    /*如果缓冲区为空，无限循环*/
    while(kbuffer.count <= 0);
    
    disable_int();
    /*现在有数据了*/
    scan_code = *(kbuffer.p_tail);
    kbuffer.p_tail ++;
    if(kbuffer.p_tail == kbuffer.buffer + KEYBOARD_BUFFER_SIZE)
    {
        kbuffer.p_tail = kbuffer.buffer;
    }
    kbuffer.count --;
    
    enable_int();
    
    return scan_code;
}

/*必须有一个任务，不断执行以下函数*/
public void keyboard_read(struct_tty *tty)
{
    unsigned int key = 0;
    unsigned int make;
    u32 *keyrow;
    unsigned int col;
    u8 scan_code;
    u8 E0 = 0;
    u8 caps;
    int i;
    
    key = 0;
    E0 = 0;
    /*扫描码的解析*/
    if(kbuffer.count > 0)
    {/*if buffer count > 0*/
        scan_code = read_byte_from_keyboard();
        
        if(scan_code == 0xE1)
        {/*if scan code == 0xE1*/
            /*判断是不是Pause/Break键*/
            unsigned char pause_code[] = {0xE1,0x1D,0x45,0xE1,0x9D,0xC5};
            u8 is_pausebreak = 1;
            
            for(i = 1 ; i < 6 ; i ++)
            {/*for*/
                if((scan_code = read_byte_from_keyboard()) != pause_code[i])
                {/*at the same times get the scan code,but it is not needed.*/
                    is_pausebreak = 0;
                    break;
                }/*end if*/
            }/*end for*/
            
            /*判断*/
            if(is_pausebreak){
                key = KEYMASK_PAUSEBREAK | PAUSEBREAK | KEYMASK_MAKED;
            }/*if the key is Pause/Break end*/
        }/*end if scan code == 0xE1*/
        else if(scan_code == 0xE0)
        {/*if scan code == 0xE0*/
            scan_code = read_byte_from_keyboard();
            
            unsigned char printscreen_make_code[] = {0xE0,0x2A,0xE0,0x37};
            unsigned char printscreen_break_code[] = {0xE0,0xB7,0xE0,0xAA};
            u8 is_printscreen_make = 1;
            u8 is_printscreen_break = 1;
            for(i = 1 ; i < 4 ; i++)
            {/*make test*/
                if(read_byte_from_keyboard() != printscreen_make_code[i])
                {/*if make test*/
                    is_printscreen_make = 0;
                    break;
                }/*end if make test*/
            }/*end for*/
            for(i = 1 ; i < 4 ; i++)
            {/*break test*/
                if(read_byte_from_keyboard() != printscreen_break_code[i])
                {/*if break test*/
                    is_printscreen_break = 0;
                    break;
                }/*end if break test*/
            }/*end for*/
            
            if(is_printscreen_make){
                key = KEYMASK_PRINTSCREEN | PRINTSCREEN | KEYMASK_MAKED;
            }else if(is_printscreen_break){
                key = KEYMASK_PRINTSCREEN | PRINTSCREEN;
            }/*end if*/
            
            /*如果不是printscreen键，则key=0*/
            if(key == 0)
            {/*otherwise,the scan code begin with e0*/
                E0 = 1;
            } /*end if key==0*/
        }/*end if scan code == 0xE0*/
        
        if(((key & 0x0ff) != PAUSEBREAK) 
            || ((key & 0x0ff) != PRINTSCREEN))
        {/*normal */
            /*首先判断是make还是break*/
            make = (scan_code & KEYMASK_BREAK)?0:1;
            keyrow = &keymap[(scan_code & 0x7f) * KEYMAP_COLS];
            caps = (flag_shift_l || flag_shift_r)?1:0;
            col = 0;
            
            if(caps_lock)
            {/*caps lock on or not*/
                if(keyrow[0] >= 'a' && keyrow[0] <= 'z')
                {/*convert only where keyrow[0] is a character.*/
                    caps = !caps;
                }/*end if*/
            }/*end if caps_lock*/
            
            if(caps)
            {/*begin if caps*/
                col = 1;
            }/*end if cpas*/
            
            if(E0)
            {/*start with E0?*/
                col = 2;
            }/*end if E0*/
            
            /*如果是Shift、Ctrl、Alt释放，需要特殊对待*/
            if(!make)
            {/*not make,release the mask*/
                if(keyrow[col] == SHIFT_L){
                    flag_shift_l = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_SHIFT_L | SHIFT_L);
                    sys_key_process(tty,KEYMASK_SHIFT_L | SHIFT_L);
                }/*end flag_shift_l*/
                if(keyrow[col] == SHIFT_R){
                    flag_shift_r = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_SHIFT_R | SHIFT_R);
                    sys_key_process(tty,KEYMASK_SHIFT_R | SHIFT_R);
                }/*end flag_shift_r*/
                if(keyrow[col] == CTRL_L){
                    flag_shift_r = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_CTRL_L | CTRL_L);
                    sys_key_process(tty,KEYMASK_CTRL_L | CTRL_L);
                }/*end flag_ctrl_l*/
                if(keyrow[col] == CTRL_R){
                    flag_shift_r = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_CTRL_R | CTRL_R);
                    sys_key_process(tty,KEYMASK_CTRL_R | CTRL_R);
                }/*end flag_ctrl_r*/
                if(keyrow[col] == ALT_L){
                    flag_shift_r = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_ALT_L | ALT_L);
                    sys_key_process(tty,KEYMASK_ALT_L | ALT_L);
                }/*end flag_alt_l*/
                if(keyrow[col] == flag_alt_r){
                    flag_shift_r = 0;
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_ALT_R | ALT_R);
                    sys_key_process(tty,KEYMASK_ALT_R | ALT_R);
                }/*end flag_alt_r*/
                
                return;
            }/*end if not make*/
            
            /*另一种情况，如果总是按下SHIFT键，则发送多个SHIFT的按键*/
            if(make)
            {/*if make and shift,ctrl,alt*/
                if(flag_shift_l && keyrow[col] == SHIFT_L)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_SHIFT_L | SHIFT_L);
                if(flag_shift_r && keyrow[col] == SHIFT_R)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_SHIFT_R | SHIFT_R);
                if(flag_ctrl_l && keyrow[col] == CTRL_L)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_CTRL_L | CTRL_L);
                if(flag_ctrl_r && keyrow[col] == CTRL_R)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_CTRL_R | CTRL_R);
                if(flag_alt_l && keyrow[col] == ALT_L)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_ALT_L | ALT_L);
                if(flag_alt_r && keyrow[col] == ALT_R)
                    sys_key_process(tty,KEYMASK_MAKED | KEYMASK_ALT_R | ALT_R);
                    
                return;
            }/*end if make and shift,ctrl,alt*/
            
            switch(keyrow[col])
            {/*test the keycode*/
                case SHIFT_L:
                    flag_shift_l = 1;
                    break;
                case SHIFT_R:
                    flag_shift_r = 1;
                    break;
                case CTRL_L:
                    flag_ctrl_l = 1;
                    break;
                case CTRL_R:
                    flag_ctrl_r = 1;
                    break;
                case ALT_L:
                    flag_alt_l = 1;
                    break;
                case ALT_R:
                    flag_alt_r = 1;
                    break;
                case CAPS_LOCK:
                    if(make)
                    {/*caps lock*/
                        caps_lock = !caps_lock;
                        set_leds();
                    }/*end if caps lock*/
                case NUM_LOCK:
                    if(make)
                    {/*num lock*/
                        num_lock = !num_lock;
                        set_leds();
                    }/*end if num lock*/
                case SCROLL_LOCK:
                    if(make)
                    {/*scroll lock*/
                        scroll_lock = !scroll_lock;
                        set_leds();
                    }/*end if scroll lock*/
                default:
                    key = keyrow[col];
            }/*end switch*/
        }/*end if*/
        
        /*如果num lock的灯亮着的话，将numpad_xx转换成字符*/
        if(num_lock && (key & KEYMASK_EXT) >= NUMPAD_DOT
            && (key & KEYMASK_EXT) <= NUMPAD_9)
            {/*num lock on*/
                key = (key & 0xffffff00) | numpad_map[(key & 0xff) - 0x2C];
            }/*end if num lock on*/
        
        if(key != 0)
        {/*if this key is effective*/
            if(key & KEYMASK_EXT == PAUSEBREAK)key = key | KEYMASK_PAUSEBREAK;
            if(key & KEYMASK_EXT == PRINTSCREEN)key = key | KEYMASK_PRINTSCREEN;
            if(scroll_lock)key = key | KEYMASK_SCROLL_LOCK;
            if(num_lock)key = key | KEYMASK_NUMLOCK;
            if(caps_lock)key = key | KEYMASK_CAPS_LOCK;
            if(flag_shift_l)key = key | KEYMASK_SHIFT_L;
            if(flag_shift_r)key = key | KEYMASK_SHIFT_R;
            if(flag_ctrl_l)key = key | KEYMASK_CTRL_L;
            if(flag_ctrl_r)key = key | KEYMASK_CTRL_R;
            if(flag_alt_l)key = key | KEYMASK_ALT_L;
            if(flag_alt_r)key = key | KEYMASK_ALT_R;
            if(make)key = key | KEYMASK_MAKED;
            
            sys_key_process(tty,key);
        }/*end if*/
    }/*end if buffer count > 0*/
}
