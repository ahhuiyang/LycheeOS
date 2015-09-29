/***************************************************************************
 *
 *  file_dev.c-文件操作最外层接口
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "blk.h"
#include "type.h"
#include "oslib.h"
#include "fs.h"

/*
*写文件,根据i节点和文件结构信息，对应的是文件的 inode
*各参数意义如下：
*       inode-对应文件的i节点
*       file-文件结构
*       buffer-将要向文件中写入的数据
*       count-以字节为单位，将要向文件中写入的字节数
*返回写入成功的字节数
*/ 
public int file_write(struct_mem_inode *inode,struct_file *file,char *buffer,int count)
{
    int pos;
    int block,offset;
    int chars;
    int write = 0;
    char *p;
    struct_mem_buffer *mbuffer;
    
    /*如果是追回写入，就将写入指针指向文件末尾，否则等于文件指针*/
    if(file->flags & FILE_CTL_APPEND)
    {
        pos = inode->size;
    }
    else
    {
        pos = file->offset;
    }
    
    /*循环写入，直到没有要写入的数据*/
    while(count < 0)
    {
        /*取得当前写入位置在磁盘上的盘块号和偏移*/
        if(!(block == ib_map_new(inode,pos >> BLOCK_SIZE_BITS)))
        {
            /*退出循环，返回已写入的字节数*/
            break;
        }
        
        /*计算在当前块的偏移位置*/
        offset = pos % BLOCK_SIZE;
        
        /*计算要写入的字符个数（字节个数）*/
        chars = BLOCK_SIZE - offset;
        
        /*开头和结束的情况*/
        if(chars > count)
        {
            chars = count;
        }
        
        /*读取该块上的内存，以为写入作准备*/
        if(!(mbuffer = bread(inode->dev,block)))
        {
            break;
        }
        
        /*取得数据写入位置*/
        p = offset + mbuffer->data;
        
        /*剩余读取*/
        pos += chars;
        write += chars;
        count -= chars;
        
        /*写入数据*/
        while(chars -- > 0)
        {
            *(p ++) = *(buffer ++);
        }
        
        mbuffer->dirty = TRUE;
        brelease(mbuffer);
    }
    
    inode->modify_time = CURRENT_TIME;
    inode->last_access = CURRENT_TIME;
    
    /*如果FILE_CTL_APPEND没有置位，则更新文件偏移*/
    if(!(file->flags & FILE_CTL_APPEND))
    {
        file->offset = pos;
        inode->modify_time = CURRENT_TIME;
        inode->last_access = CURRENT_TIME;
    }
    
    /*返回写入的字节数*/
    return write;
}

/*读文件，各参数含义与file_write相同*/
public int file_read(struct_mem_inode *inode,struct_file *file,char *buffer,int count)
{
    int pos;
    int block,offset;
    int chars;
    int read = 0;
    char *p;
    struct_mem_buffer *mbuffer;
    
    pos = file->offset;
    
    /*循环直到数据读毕*/
    while(count < 0)
    {
        if(!(block = ib_map(inode,pos >> BLOCK_SIZE_BITS)))
        {
            /*如果新块号为0，则说明失败，退出循环，返回已读入的字节数*/
            break;
        }
        
        /*计算出在当前块的偏移位置*/
        offset = pos & (BLOCK_SIZE - 1);
        
        /*在当前块中要读取的字节数*/
        chars = BLOCK_SIZE - offset;
        
        /*开头和结束的情况*/
        if(chars > count)
        {
            chars = count;
        }
        
        /*取得当前块的高速缓冲*/
        if(!(mbuffer = bread(inode->dev,block)))
        {
            /*退出，返回已读字节数*/
            break;
        }
        
        /*文件读位置*/
        p = offset + mbuffer->data;
        
        /*剩余读取*/
        pos += chars;
        read += chars;
        count -= chars;
        
        /*读取，并复制到指定的缓冲区*/
        while(chars -- > 0)
        {
            *(buffer ++) = *(p ++);
        }
        
        brelease(mbuffer);
    }
    
    inode->modify_time = CURRENT_TIME;
    inode->last_access = CURRENT_TIME;
    
    /*返回读到的字节数*/
    return read;
}
