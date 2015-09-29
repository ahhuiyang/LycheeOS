/***************************************************************************
 *
 *  super.c-对超级块进行处理
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "type.h"
#include "string.h"
#include "process.h"
#include "fs.h"

/*内存中的超级块数组，共8项*/
struct_mem_super_block super_block[NR_SUPER];

/*锁定指定的超级块*/
private void lock_super(struct_mem_super_block *sb)
{
    while(sb->locked)
    {
        sleep_on(&sb->proc_wait);
    }
    
    /*等待成功*/
    sb->locked = TRUE;
}

/*对指定超级块解锁*/
private void unlock_super(struct_mem_super_block *sb)
{
    sb->locked = FALSE;
    wake_up(&sb->proc_wait);
}

/*睡眠等待超级块解锁*/
private void wait_on_super(struct_mem_super_block *sb)
{
    while(sb->locked)
    {
        sleep_on(&sb->proc_wait);
    }
}

/*取指定设备上的超级块*/
public struct_mem_super_block *get_super(int dev)
{
    struct_mem_super_block *temp;
    
    temp = super_block;
    while(temp < NR_SUPER + super_block)
    {
        if(temp->dev == dev)
        {
            wait_on_super(temp);
            /*等待过后需要重新检查*/
            if(temp->dev == dev)
            {
                return temp;
            }
            /*如果等待过后设备号不同了，则重新搜索*/
            temp = super_block;
        }
        else
        {
            temp ++;
        }
    }
    
    /*到这里，说明找不到超级块*/
    return NULL;
}

/*  释放指定设备的超级块，并释放该设备的i节点位图和数据块位图
*   如果超级块对应的文件系统是根文件系统，或者i节点上已经安装有其他的文件系统
*   则不释放
*/
public void put_super(int dev)
{
    struct_mem_super_block *sb;
    struct_mem_inode *inode;
    int i;
    
    /*不能释放根设备*/
    if(dev == sys_root_dev)
    {
        printfs("can not release root dev.\n");
        return;
    }
    
    /*获取超级块，如果无法获取，则返回*/
    if(!(sb = get_super(dev)))
    {
        printfs("can not get super of dev %d\n",dev);
        halt();
    }
    
    /*如果该文件系统安装到的i节点没有被处理，则不在此处处理
    *否则就是卸载文件系统的功能了，因此这里直接返回
    */
    if(sb->imount)
    {
        return;
    }
    
    /*现在开始释放超级块，要释放i节点位图和数据块位图*/
    lock_super(sb);
    
    /*设置超级块的设备号为零，以表明此超级块不用了*/
    sb->dev = 0;
    /*释放i节点位图*/
    for(i = 0 ; i < INODE_MAP_SLOTS ; i ++)
    {
        if(sb->imap[i])
        {
            brelease(sb->imap[i]);
        }
    }
    
    for(i = 0 ; i < DATA_MAP_SLOTS ; i ++)
    {
        if(sb->dmap[i])
        {
            brelease(sb->dmap[i]);
        }
    }
    
    unlock_super(sb);
}

/*从设备上读取超级块*/
private struct_mem_super_block *read_super(int dev)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int i,block;
    
    /*如果设备为空，则退出*/
    if(!dev)
    {
        return NULL;
    }
    
    /*如果超级块已经存在内存，则直接返回*/
    if(sb = get_super(dev))
    {
        return sb;
    }
    
    /*否则，先找一个空超级块，然后将设备上的超级块读进来*/
    for(i = 0 ; i < NR_SUPER ; i ++)
    {
        if(!super_block[i].dev)
        {
            sb = super_block + i;
            break;
        }
    }
    
    /*如果没有找到空超级块，则返回空值*/
    if(i >= NR_SUPER)return NULL;
    /*下面要对超级块操作了*/
    lock_super(sb);
    /*从设备中读取数据*/
    if(!(mbuffer = bread(dev,FS_POS_SUPER)))
    {
        sb->dev = 0;
        unlock_super(sb);
        return NULL;
    }
    
    /*初始化超级块内容*/
    *((struct_super_block *)sb) = 
        *((struct_super_block *)mbuffer->data);
    /*释放内存缓冲*/
    brelease(mbuffer);
    /*通过magic number判断所读数据是否正确*/
    if(sb->magic != SUPER_MAGIC)
    {
        sb->dev = 0;
        free_super(sb);
        return NULL;
    }
    
    /*对超级块内存中的成员进行初始化*/
    sb->dev = dev;
    sb->root_inode = NULL;
    sb->imount = null;
    sb->proc_wait = NULL;
    sb->locked = false;
    sb->read_only = false;
    sb->dirty = 0;
    sb->uptodate = 1;
    for(i = 0 ; i < INODE_MAP_SLOTS ; i ++)
    {
        sb->imap[i] = NULL;
    }
    for(i = 0 ; i < DATA_MAP_SLOTS ; i ++)
    {
        sb->dmap[i] = NULL;
    }
    
    /*读取位图*/
    block = 2;
    for(i = 0 ; i < sb->imap_blocks ; i ++)
    {
        if(sb->imap[i] = bread(dev,block))
        block ++;
    }
    for(i = 0 ; i < sb->dmap_blocks ; i ++)
    {
        sb->dmap[i] = bread(dev,block);
        block ++;
    }
    
    unlock_super(sb);
    
    return sb;
}
