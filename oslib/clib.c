/***************************************************************************
 *
 *  clib.c-常用函数库
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "../include/type.h"
#include "../include/io.h"

public u8 read_cmos(u8 addr)
{  
    if(addr < 0 || addr >127)
    {
        return -1;
    }
    out_byte(addr,0x70);
    return in_byte(0x71);
}
