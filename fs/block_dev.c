/***************************************************************************
 *
 *  block_dev.c-块设备文件系统接口
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "blk.h"
#include "type.h"
#include "fs.h"
#include "kernel.h"

/*
*   块写函数，这个函数是对设备文件来说的，并不是普通文件
*   写入操作并不是直接将数据写入设备，而是写入内存高速缓冲
*   各参数意义如下：
*       dev-完整的设备号
*       pos-设备文件中的数据偏移位置，这个偏移位置对应相同的磁盘位置
*       buffer-要读入的数据缓冲区
*       count-要写入的字节个数
*   函数返回写入成功的字节数
*/
public int block_write(int dev,int *pos,char *buffer,int count)
{
    /*数据偏移对应到磁盘上的盘块号*/
    int block = *pos >> BLOCK_SIZE_BITS;
    /*偏移位置*/
    int offset = *pos & (BLOCK_SIZE - 1);
    /*在一块中可写入字节数*/
    int chars;
    /*写入的字节数*/
    int write = 0;
    /*由于实际上先向内存缓冲中写入，所以要有一个缓冲区指针*/
    struct_mem_buffer *mbuffer;
    /*指向当前写入位置*/
    char *p;
    
    /*现在开始写数据，方法是：
    *   顺序写入
    *   处理开始和结束时的不成块数据
    */
    
    /*条件：只要还有要写的数据*/
    while(count > 0)
    {
        /*算出在当前块要写入的字节数*/
        chars = BLOCK_SIZE - offset;
        /*如果在一块上要写入的字节数比总的字节数小，则要写入的字节数为总字节数
        *这种情况包括开始的不足一块数据和结束时的不足一块数据
        */
        if(chars > count)
        {
            chars = count;
        }
        /*获取指定设备和块号的缓冲块*/
        if(!(mbuffer = bread(dev,block)))
        {
            return write;
        }
        /*变量p指向要写入的位置*/
        p = offset + mbuffer->data;
        /*重置offset，因为在接下来的数据写入过程中，offset只是块的起始*/
        block ++;
        offset = 0;
        
        /*先更新变量的值，再写入*/
        *pos += chars;
        write += chars;
        count -= chars;
        
        /*开始写数据*/
        while(chars -- > 0)
        {
            *(p ++) = *(buffer ++);
        }
        
        mbuffer->dirty = TRUE;
        brelease(mbuffer);
    }
    
    /*返回写入字节的个数*/
    return write;
}

/*
*块读函数，从指定设备和指定位置读入指定字节数的数据到缓冲中
*各参数的含义同block_write函数相同
*/
public int block_read(int dev,int *pos,char *buffer,int count)
{
    /*要读出的位置在设备上的块号*/
    int block = *pos >> BLOCK_SIZE_BITS;
    /*要读出的位置在设备上的偏移*/
    int offset = *pos & (BLOCK_SIZE - 1);
    /*在一块上要写入的字节数*/
    int chars;
    /*读取的字节数*/
    int read = 0;
    /*高速缓冲块*/
    struct_mem_buffer *mbuffer;
    /*当前要读取的位置*/
    char *p;
    
    /*循环条件：直到没有要读出的数据*/
    while(count > 0)
    {
        /*计算当前块可以读出的字节数*/
        chars = BLOCK_SIZE - offset;
        /*如果可读出的字节数比需要读出的字节数大*/
        if(chars > count)
        {
            chars = count;
        }
        
        /*取得当前设备和块号的内存高速缓冲块*/
        if(!(mbuffer = bread(dev,block)))
        {
            return read;
        }
        
        /*设备读位置*/
        p = offset + mbuffer->data;
        
        /*指向下一块，并设置下一次的偏移位置*/
        block ++;
        offset = 0;
        
        /*剩余读取*/
        *pos += chars;
        read += chars;
        count -= chars;
        
       /*读取，并复制到缓冲区*/
       while(chars -- > 0)
       {
        *(buffer ++) = *(p ++);
       }
       
       /*释放上一个缓冲块，留着下一次用*/
       brelease(mbuffer);
    }
    
    /*返回读到的字节数*/
    return read;
}
