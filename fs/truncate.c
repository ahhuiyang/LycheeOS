/***************************************************************************
 *
 *  truncate.c-释放指定i节点在设备上占用的所有逻辑块
 *  包括直接块、一次间接块和二次间接块，从而将文件对应的
 *  长度截为零，并释放占用的设备空间
 *
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "type.h"
#include "blk.h"
#include "process.h"
#include "fs.h"

/*释放一次间接块，block是间接块的块号*/
private void free_level1(int dev,int block)
{
    struct_mem_buffer *mbuffer;
    int i;
    u16 *p;
    
    /*块号不能为零*/
    if(!block)
    {
        return;
    }
    
    /*下面释放一次间接块中指向的数据块号*/
    if(mbuffer = bread(dev,block))
    {
        p = (u16 *)mbuffer->data;
        for(i = 0 ; i < (BLOCK_SIZE / sizeof(u16)) ; i ++)
        {
            if(*p)
            {
                free_block(dev,*p);
            }
        }
        brelease(mbuffer);
    }
    
    /*最后释放一次间接块*/
    free_block(dev,block);
}

/*释放二次间接块，block是二次间接块的块号*/
private void free_level2(int dev,int block)
{
    struct_mem_buffer *mbuffer;
    int i;
    u16 *p;
    
    /*块号不能为零*/
    if(!block)
    {
        return;
    }
    
    if(mbuffer = bread(dev,block))
    {
        p = (u16 *)mbuffer->data;
        
        /*下面循环释放多个可以看作为一次间接块的块*/
        for(i = 0 ; i <= BLOCK_SIZE / sizeof(u16) ; i ++)
        {
            if(*p)
            {
                free_level1(dev,*p);
            }
        }
        
        brelease(mbuffer);
    }
    
    /*最后再释放二次间接块本身*/
    free_block(dev,block);
}

/*将文件截断为零，并释放占用的空间*/
/*由于此函数供文件系统本身调用，所以不考虑竞争条件*/
public void truncate(struct_mem_inode *inode)
{
    int i;
    
    /*参数合法性检查*/
    if(!inode)
    {
        return;
    }
    
    /*判断是不是目录文件或普通文件，只有这两种文件才可以进行操作*/
    if(IS_DIR(inode->mode) || IS_NORMAL(inode->mode))
    {
        /*首先释放直接块号*/
        for(i = 0 ; i < INDEX_LEVEL1 ; i ++)
        {
            free_block(inode->dev,inode->data[i]);
            inode->data[i] = 0;
        }
        
        /*一次间接块*/
        free_level1(inode->dev,inode->data[INDEX_LEVEL1]);
        free_level2(inode->dev,inode->data[INDEX_LEVEL2]);
        inode->data[INDEX_LEVEL1] = 0;
        inode->data[INDEX_LEVEL2] = 0;
        
        /*文件长度为0*/
        inode->size = 0;
        /*已修改inode*/
        inode->dirty = TRUE;
        /*时间*/
        inode->modify_time = CURRENT_TIME;
        inode->last_access = CURRENT_TIME;
    }
}
