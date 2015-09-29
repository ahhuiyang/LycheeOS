/***************************************************************************
 *
 *  rw.c-文件系统与外界的接口，read和write函数
 *  以系统调用的形式向外界提供功能
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "type.h"
#include "kernel.h"
#include "process.h"
#include "fs.h"
#include "aifs.h"

/*
*系统调用
*下面两个是相当重要的系统调用
*文件系统向用户暴露的接口，通过系统调用向外部提供功能
*所谓的文件句柄，实际上是其在进程结构的文件数组中的下标
*
*返回值：成功－返回写字节数，失败－返回0
*/
public int sys_write(u32 fhandle,char *buffer,int count)
{
    struct_file *file;
    struct_mem_inode *inode;
    
    /*首先判断文件句柄是否合法，如果不合法，则返回0*/
    if(fhandle >= NR_OPEN || count <=0 
        || !(file = current->files[fhandle]))
    {
        return 0;
    }
    
    /*取得文件的内存i节点*/
    inode = file->inode;
    
    /*下面开始写分发过程，即按照文件类型的不同，写具体的设备或文件*/
    if(IS_CHR(inode->mode))
    {
        return chr_write(inode->data[0],buffer,count);
    }
    else if(IS_BLK(inode->mode))
    {
        return block_write(inode->data[0],&file->offset,buffer,count);
    }
    else if(IS_DIR(inode->mode) || IS_NORMAL(inode->mode))
    {
        return file_write(inode,file,buffer,count);
    }
    
    /*能执行到这里，说明有错误，返回零*/
    return 0;
}

/*
*系统调用
*文件系统读调用
*/
public int sys_read(u32 fhandle,char *buffer,int count)
{
    struct_file *file;
    struct_mem_inode *inode;
    
    /*判断文件句柄和读取数是否合理*/
    if(fhandle >= NR_OPEN || count <= 0 || !(file = current->files[fhandle]))
    {
        return 0;
    }
    
    inode = file->inode;
    
    /*
    *下面是读的主体，依据文件类型的不同，调用对应的读函数
    */
    if(IS_CHR(inode->mode))
    {
        return chr_read(inode->data[0],buffer,count);
    }
    else if(IS_BLK(inode->mode))
    {
        return blk_read(inode->data[0],&file->offset,buffer,count);
    }
    else if(IS_DIR(inode->mode) || IS_NORMAL(inode->mode))
    {
        return file_read(inode,file,buffer,count);
    }
}
