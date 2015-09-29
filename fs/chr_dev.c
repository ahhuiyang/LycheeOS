/***************************************************************************
 *
 *  chr_dev.c-文件操作函数
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/

#include "tty.h"
#include "kernel.h"
#include "blk.h"

/*字符写，其实质是往显示器上显示文字，返回写入字符数*/
public int chr_write(int dev,char *buffer,int count)
{
    if(MAJOR(dev) == MAJOR_TTY)
    {
        return tty_write(MINOR(dev),buffer,count);
    }
}

/*字符读，其实质是从键盘读数据，返回读入字符数*/
public int chr_read(int dev,char *buffer,int count)
{
    if(MAJOR(dev) == MAJOR_TTY)
    {
        return tty_read(MINOR(dev),buffer,count);
    }
}
