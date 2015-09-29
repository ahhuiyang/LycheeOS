/***************************************************************************
 *
 *  io.h-通用IO端口处理宏
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/

#ifndef _IO_H
#define _IO_H

/*字节输出，对字的输出也是可以的*/
#define out_byte(value,port) \
    __asm__("outb %%al,%%dx"::"a"(value),"d"(port))
    
/*字节读取函数，对字的读取也应该是可以的*/
#define in_byte(port)({\
    unsigned char _c;\
    __asm__ volatile("inb %%dx,%%al":"=a"(_c):"d"(port));\
    _c;\
})

/*带延迟的端口字节输入输出函数*/
#define delay_out_byte(value,port)\
    __asm__("out %%al,%%dx\n"\
            "\tnop\n"\
            "1:\tnop"::"a"(value),"d"(port))

#define delay_in_byte(port)({\
    unsigned char _c;\
    __asm__ volatile("in %%dx,%%ax\n"\
                     "\tnop\n"\
                     "\tnop\n":"=a"(_c):"d"(port));\
    _c;\
})

/*从端品读数据，读n字，放到指定缓冲区中*/
#define port_read(port,buffer,nr)\
    __asm__("cld;rep;insw"::"d"(port),"D"(buffer),"c"(nr))
    
/*将指定缓冲区数据写入端口，写n字*/
#define port_write(port,buffer,nr)\
    __asm__("cld;rep;outsw"::"d"(port),"S"(buffer),"c"(nr))

#endif

