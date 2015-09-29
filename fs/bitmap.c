/***************************************************************************
 *
 *  bitmap.c-对inode节点位图和数据块位图进行操作的函数
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "string.h"
#include "blk.h"
#include "fs.h"
#include "buffer.h"

/*释放指定数据块，如果参数超出数据块范围，则退出*/
public void free_block(int dev,int block)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    
    if(!(sb = get_super(dev)))
    {
        printfs("try to free super block on nonexistent device.\n");
        halt();
    }
    
    /*数据块号不能超出范围*/
    if(block < sb->first_data_block || block >= sb->nr_blocks)
    {
        printfs("try to free block not in data zone.\n");
        halt();
    }
    
    /*如要当前数据块在对应的内存缓冲中，则释放对应的缓冲块*/
    mbuffer = find_buffer(dev,block);
    if(mbuffer)
    {
        if(mbuffer->count != 1)
        {
            printfs("try to free block(dev=%d,block=%d),count=%d",dev,block,mbuffer->count);
            return;
        }
        
        mbuffer->dirty = 0;
        mbuffer->uptodate = 0;
        brelease(mbuffer);
    }
    
    /*下面清理数据块位图*/
    int index = block - sb->first_data_block + 1;
    if(!(clear_bit(sb->dmap[index / 8192],index & 8191)))
    {
        printfs("dev=%d,block=%d was already cleared.\n",dev,block);
        halt();
    }
    sb->dmap[index / 8192]->dirty = TRUE;
    
}

/*从设备申请一个逻辑块，返回盘块号，如果没有可用盘块号，则返回0*/
int new_block(int dev)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int i,block;
    
    if(!(sb = get_super(dev)))
    {
        printfs("try to free super block on nonexistent device.\n");
        halt();
    }
    
    /*扫描数据块位图*/
    for(i = 0 ; i < 8 ; i ++)
    {
        if(mbuffer = sb->dmap[i])
        {
            if((block = find_first_zero(mbuffer->data)) < 8192)
                break;
        }
    }
    
    /*判断是否找到*/
    if(i >= 8 || (!mbuffer) || block >= 8192)
    {
        return 0;       /*没有找到*/
    }
    
    /*设备新块的逻辑位图比特位，如果原位为1，则出错*/
    if(set_bit(mbuffer->data,block))
    {
        printfs("bitmap.c:new_block-data map bit already set.\n");
        halt();
    }
    
    /*置对应缓冲块已修改*/
    mbuffer->dirty = 0;
    
    /*判断新申请的块号是否大于总块数*/
    block = i * 8192 + block + sb->first_data_block - 1;
    
    /*如果新块号大于或等待磁盘总数，则说明有问题，返回0，表示查找失败*/
    if(block >= sb->nr_blocks)
    {
    return;
    }
    
    /*下面要对这块数据进行初始化*/
    if(!(mbuffer = get_buffer(dev,block)))
    {
        printfs("dev=%d,block=%d,cannot get block.\n",dev,block);
        halt();
    }
    
    /*如果引用计数大于1，则出错死机*/
    if(mbuffer->count != 1)
    {
        printfs("dev=%d,block=%d:count = %d",dev,block,mbuffer->count);
        halt();
    }
    
    /*清空数据*/
    clear_block(mbuffer->data);
    mbuffer->dirty = 1;             /*表示缓冲中的数据修改了，要写回设备*/
    mbuffer->uptodate = 1;          /*表示现在缓冲中的数据有效，不要从设备读了*/
    /*释放缓冲区，以使数据写回设备*/
    brelease(mbuffer);
}

/*释放指定的i节点，这相当于删除某个目录或文件*/
public void free_inode(struct_mem_inode *inode)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int i,j;
    
    /*检查参数的合法性*/
    if(!inode)
    {
        return;
    }
    
    /*如果设备号为零，则说明此i节点无用*/
    if(!inode->dev)
    {
        memset(inode,0,sizeof(*inode));
        return;
    }
    
    /*如果此i节点还有其它引用，则不能释放，说明内核有问题*/
    if(inode->count > 1)
    {
        printfs("try to free inode with count=%d",inode->count);
        halt();
    }
    
    /*检查是否还有目录指定此节点*/
    if(inode->nlinks)
    {
        printfs("try to free inode with nlinks=%d",inode->nlinks);
        halt();
    }
    
    /*获得超级块*/
    if(!(sb = get_super(inode->dev)))
    {
        printfs("cannot access super block of dev=%d",inode->dev);
        halt();
    }
    
    /*检查i节点号是否超出范围*/
    if(inode->inode_num < 1 || inode->inode_num >= sb->nr_inodes)
    {
        printfs("try to free inode 0 or nonexistent inode.\n");
        halt();
    }
    
    /*如果对应的高速缓冲不存在，则出错*/
    if(!(mbuffer = sb->imap[inode->inode_num / 8192]))
    {
        printfs("inode map does not exist.\n");
        halt();
    }
    
    /*清除i节点位图对应标志位*/
    if(!(clear_bit(mbuffer->data,inode->inode_num & 8191)))
    {
        printfs("dev=%d,inode num = %d bit is already cleared.\n",
            inode->dev,inode->inode_num);
        halt();
    }
    
    mbuffer->dirty = TRUE;
    memset(inode,0,sizeof(*inode));
}

/*从设备申请一个inode，相当于新建目录或文件*/
public struct_mem_inode *new_inode(int dev)
{
    struct_mem_inode *inode;
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int i,j;
    
    /*
    *首先在内存中获取一个空闲节点，指的是从i节点表中获取
    *并不是从磁盘上获取
    */
    if(!(inode = get_empty_inode()))
    {
        return NULL;
    }
    
    /*获取相应设备的超级块*/
    if(!(sb = get_super(dev)))
    {
        halts("can not get super in function new_inode().\n");
    }
    
    /*在i节点位图中查找空闲节点*/
    j = 8192;
    for(i = 0 ; i < 8 ; i ++)
    {
        if(mbuffer = sb->imap[i])
        {
            if((j = find_first_zero(mbuffer->data)) < 8192)
            {
                break;
            }
        }
    }
    
    if(j + i * 8192 >= sb->nr_inodes || (!mbuffer) || j>=8192)
    {
        put_inode(inode);
        return NULL;
    }
    
    /*设置i节点位图*/
    if(set_bit(mbuffer->data,j))
    {
        /*如果此位已设置，说明内核代码中有错。*/
        halts("bit is already set in function new_inode().\n");
    }
    
    /*内存缓冲已修改了，置位脏标志*/
    mbuffer->dirty = TRUE;
    /*初始化新申请到的节点*/
    inode->modify_time = inode->create_time = inode->last_access = CURRENT_TIME;
    inode->nlinks = 1;
    inode->proc_wait = NULL;
    inode->dev = dev;
    inode->inode_num = j + i * 8192;
    inode->count = 1;
    inode->locked = FALSE;
    inode->dirty = TRUE;
    inode->uptodate = TRUE;
    inode->mount = FALSE;
    
    return inode;
}
