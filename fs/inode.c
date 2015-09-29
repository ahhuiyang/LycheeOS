/***************************************************************************
 *
 *  inode.c-对inode节点进行处理
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
#include "type.h"
#include "io.h"
#include "string.h"
#include "process.h"
#include "fs.h"
#include "blk.h"

/*pre declaratio*/
private void write_inode(struct_mem_inode *inode);
private void read_inode(struct_mem_inode *inode);

/*内存中的inode数组，用于临时存放inode节点*/
public struct_mem_inode inode_table[NR_INODE];

/*等待inode解锁*/
private void wait_on_inode(struct_mem_inode *inode)\
{
    while(inode->locked)
    {
        sleep_on(&inode->proc_wait);
    }
}

/*对指定的i节点上锁*/
private void lock_inode(struct_mem_inode *inode)
{
    while(inode->locked)
    {
        sleep_on(&inode->proc_wait);
    }
    inode->locked = TRUE;
}

/*解锁指定的i节点*/
private void unlock_inode(struct_mem_inode *inode)
{
    inode->locked = FALSE;
    wake_up(&inode->proc_wait);
}

/*释放内存中，设备dev相关的所有i节点*/
public void invalid_inodes(int dev)
{
    int i;
    struct_mem_inode *inode;
    
    for(i = 0 ; i < NR_INODE ; i ++)
    {
        inode = &inode_table[i];
        wait_on_inode(inode);
        if(inode->dev == dev)
        {
            /*如果某个i节点仍存使用，则显示信息*/
            if(inode->count)
            {
                printfs("dev=%d,inode %d is still used.\n",dev,inode->inode_num);
            }
            
            /*释放，方法是置dev和dirty标志都为零或FALSE*/
            inode->dev = inode->dirty = 0;
        }
    }
}

/*同步所有i节点。即保持盘上的数据和内存中的数据一致，将i节点写入磁盘*/
public void sync_inodes(void)
{
    int i;
    struct_mem_inode *inode;
    
    for(i = 0 ; i < NR_INODE ; i ++)
    {
        inode = &inode_table[i];
        wait_on_inode(inode);
        
        /* 如果有已修改标志，则写入磁盘*/
        if(inode->dirty)
        {
            write_inode(inode);
        }
    }
}

/*
*       将文件数据块映射到盘块，返回设备上的物理盘块号，0表示失败
*       文件数据块是按顺序编号的
*       然后映射到不按顺序的物理盘块
*       映射关系是从前往后，逐一映射
*/
private int ib_map_helper(struct_mem_inode *inode,int block,int new)
{
    struct_mem_buffer *mbuffer;
    int i;
    
    /*如果参数不合法，则返回0表示失败*/
    if(!inode || block < 0 || block > 7 + 512 + 512 * 512)
    {
        return 0;
    }
    
    /*由于有直接块号、一级块号和二级块号，所以按情况判断*/
    if(block < 7)
    {
        if(new && (!inode->data[block]))
        {
            inode->data[block] = new_block(inode->dev);
            if(inode->data[block])
            {
                inode->modify_time = CURRENT_TIME;
                inode->last_access = CURRENT_TIME;
            }
        }
        return inode->data[block];
    }
    else if(block < 7 + 512)
    {
        if(new && (!inode->data[INDEX_LEVEL1]))
        {
            inode->data[INDEX_LEVEL1] = new_block(inode->dev);
            if(inode->data[INDEX_LEVEL1])
            {
                inode->dirty = 1;
                inode->modify_time = CURRENT_TIME;
                inode->last_access = CURRENT_TIME;
            }
            
                /*如果盘块号为空，则返回0表示失败*/
            if(!inode->data[INDEX_LEVEL1])
            {
                return 0;
            }
            
            /*现在读取一次间接块，并返回指定的盘块号*/
            if(!(mbuffer = bread(inode->dev,inode->data[INDEX_LEVEL1])))
            {
                /*读取失败，则返回0*/
                return 0;
            }
            
            i = ((u16 *)(mbuffer->data))[block - 7];
            if(new && !i)
            {
                i = new_block(inode->dev);
                if(i)
                {
                    /*一次间接块在内存高速缓冲中的数据修改了，所以要更改其修改标志*/
                    ((unsigned short *)(mbuffer->data))[block] = i;
                    mbuffer->dirty = TRUE;
                }
            }
        }
        
        brelease(mbuffer);
        return i;
    }
    else        /*block位于二级盘块位置*/
    {
        block -= (7 + 512);
        /*如果二次盘块号不存在，并且新建标志置位，则新申请一个盘块号*/
        if(new && !inode->data[INDEX_LEVEL2])
        {
            inode->data[INDEX_LEVEL2] = new_block(inode->dev);
            if(inode->data[INDEX_LEVEL2])
            {
                inode->dirty = TRUE;
                inode->modify_time = CURRENT_TIME;
                inode->last_access = CURRENT_TIME;
            }
        }
        
        /*如果二级缓冲块号为0，则表示没有二级缓冲块，返回0表示没找到*/
        if(!inode->data[INDEX_LEVEL2])
        {
            return 0;
        }
        
        /*读取二级缓冲块*/
        if(!(mbuffer = bread(inode->dev,inode->data[INDEX_LEVEL2])))
        {
            return 0;
        }
        
        /*然后从中获取下一级缓冲块所在的磁盘块号*/
        i = ((u16 *)mbuffer->data)[block >> 9];
        
        /*然后读取或创建盘块号码*/
        if(new && (!i))
        {
            /*申请一个新的盘块*/
            i = new_block(inode->dev);
            if(i)
            {
                ((u16 *)mbuffer->data)[block >> 9] = i;
                inode->modify_time = CURRENT_TIME;
                inode->last_access = CURRENT_TIME;
                inode->dirty = TRUE;
            }
        }
        brelease(mbuffer);
        
        /*如果i为零，返回0*/
        if(!i)return 0;
        
        /*读取二级缓冲块*/
        if(!(mbuffer = bread(inode->dev,i)))
        {
            return 0;
        }
        
        /*然后从中获取下一级缓冲块所在的磁盘块号*/
        i = ((u16 *)mbuffer->data)[block & 511];
        
        /*如果新建标志置位且要查找的盘块为零，则新建盘块*/
        if(new && !i)
        {
            i = new_block(inode->dev);
            if(i)
            {
                ((u16 *)mbuffer->data)[block & 511] = i;
                inode->modify_time = CURRENT_TIME;
                inode->last_access = CURRENT_TIME;
                inode->dirty = TRUE;
            }
        }
        
        brelease(mbuffer);
        return i;
    }
}

/*将按顺序编号的文件块号翻译为物理盘块号，inode指文件，block是文件盘块编号*/
public int ib_map(struct_mem_inode *inode,int block)
{
    return ib_map_helper(inode,block,0);
}

/*与上一个函数功能相同，不同之处是若物理盘块号不存在，同创建它*/
public int ib_map_new(struct_mem_inode *inode,int block)
{
    return ib_map_helper(inode,block,1);
}

/*释放一个inode节点，即将节点内容回写入设备*/
public void put_inode(struct_mem_inode *inode)
{
    /*检验参数的合法性*/
    if(!inode)
    {
        return;
    }
    
    /*等待i节点解锁*/
    wait_on_inode(inode);
    
    /*如果当前引用计数为零，说明当前的inode没被使用，内核代码有问题，死机*/
    if(!inode->count)
    {
        printfs("try to release free inode.\n");
        halt();
    }
    
    /*如果设备号为零，则返回*/
    if(!inode->dev)
    {
        return;
    }
    
    /*如果此i节点代表的是块设备文件，则同步块设备*/
    if(IS_BLK(inode->mode))
    {
        /*同步这个块设备*/
        sync_dev(inode->data[0]);
        /*等待同步完成*/
        wait_on_inode(inode);
    }
    
    /*开始进行i节点的释放*/
    while(TRUE)
    {
        /*如果i节点的引用计数大于1，说明还有其它进程引用，不能释放*/
        /*将当前的引用计数减1*/
        if(inode->count > 1)
        {
            inode->count --;
            return;
        }
        
        /*如果没有目录指向当前的i节点，则说明当前的i节点没有用了，删除它*/
        if(!inode->nlinks)
        {
            truncate(inode);
            free_inode(inode);
            return;
        }
        
        /*如果inode修改过，则要写回磁盘*/
        if(inode->dirty)
        {
            write_inode(inode);
            /*等待写完*/
            wait_on_inode(inode);
            /*等待过后，需要重新判断*/
            continue;
        }
        
        /*若能执行到此，说明inode->count = 1 , inode->dirty = FALSE*/
        /*将inode->count*置0，则此节点可供其它进程命名用了*/
        inode->count --;
        break;
    }
}

/*从i节点表(inode_table)中获取一个空闲的i节点*/
public struct_mem_inode *get_empty_inode(void)
{
    struct_mem_inode *inode;
    int i;
    
    /*下面开始寻找节点*/
    while(TRUE)
    {
        inode = NULL;
        
        for(i = 0 ; i < NR_INODE ; i ++)
        {
            /*
            *如果某个节点引用计数为零，则说明没有进程再使用此节点
            *此节点可能是可用的
            */
         if(!inode_table[i].count)
            {
                /*如果节点没有上锁，并且不脏，则可能找到了*/
                if((!inode_table[i].dirty) && (inode_table[i].locked))
                {
                    inode = &inode_table[i];
                    break;
                }           
            }
        }
        
        /*如果没有足够的节点，则说明内核有问题，对资源管理不善，则死机*/
        if(!inode)
        {
            halts("no enough free inodes in memory.\n");
        }
        
        /*等待其解锁*/
        wait_on_inode(inode);
        /*如果有脏标志，则回写*/
        while(inode->dirty)
        {
            write_inode(inode);
            wait_on_inode(inode);
        }
        
        /*再判断一下引用计数，不为零则重新寻找*/
        if(!inode->count)break;
    }
    
    /*到这里，我们找到了所需要的节点*/
    memset(inode,0,sizeof(*inode));
    inode->count = 1;
    return inode;
}

/*从设备(dev)上读取指定节点(nr)的i节点到内存中
*如果已经在内存中了，并且不是安装点，则返回找到的节点
*如果是安装点，则查找被安装文件根i节点，如果找到，则返回被安装文件系统根节点
*如果查找的节点不在内存，则查找一个空闲节点，并从磁盘读取相应节点
*
*可能会不一致：
*      如果当前安装点不在内存，只会读取安装点
*      如果在内存，会返回被安装文件系统的根i节点
*
*可以在外层函数里避免这种不一致，但应该是这个函数的问题
*/
public struct_mem_inode *get_inode(int dev,int nr)
{
    struct_mem_inode *inode,iempty;
    int i;
    
    /*参数合法检查*/
    if(dev <= 0)
    {
        return;
    }
    
    /*首先查找存中节点表中是否有要获得的i节点*/
    inode = inode_table;
    while(inode < inode_table + NR_INODE)
    {
        /*如果设备号和节点号对不上，则检查下一个*/
        if(inode->dev != dev || inode->inode_num != nr)
        {
            inode ++;
            continue;
        }
        
        /*找到了指定设备和节点号的inode，等待其解锁*/
        wait_on_inode(inode);
        
        /*等待之后，设备号和i节点号可能会改变，因此再判断一下*/
        if(inode->dev != dev || inode->inode_num != nr)
        {
            /*如果改变了，则重新扫描*/
            inode = inode_table;
            continue;
        }
        
        /*递增引用计数*/
        inode->count ++;
        
        /*如果inode是安装点，则读取被安装文件系统根i节点*/
        if(inode->mount)
        {
            /* 查找超级块，其安装inode等待当前查找到的i节点*/
            for(i = 0 ; i < NR_SUPER ; i ++)
            {
                if(super_block[i].imount == inode)
                {
                    break;
                }
            }
            
            /*如果没找到的话，直接返回上一次找到的i节点*/
            if(i >= NR_SUPER)
            {
                printfs("mount inode doesn't have root.\n");
                return inode;
            }
            
            /*找到了超级块，继续查找被安装文件系统根节点*/
            iput(inode);
            dev = super_block[i].dev;
            nr = ROOT_INODE;
            inode = inode_table;
            continue;
        }
        
        /*在内存中找到了i节点，返回*/
        return inode;
    }
    
    /*如果内存中没有找到，则新申请一个*/
    if(!(inode = get_empty_inode()))
    {
        inode->dev = dev;
        inode->inode_num = nr;
        inode->uptodate = FALSE;
        read_inode(inode);
        
        return inode;
    }
    
    /*在内存中没有找到，也没找到空闲结点，则返回空*/
    return NULL;
}

/*从设备上读取指定节点的i节点信息到内存中*/
private void read_inode(struct_mem_inode *inode)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int block;
    
    /*首先锁定该节点，取得所在设备的超级块，以得到该节点的具体设备位置*/
    lock_inode(inode);
    
    /*如果结点数据有效，则不读取*/
    if(inode->uptodate)
    {
        unlock_inode(inode);
        return;
    }
    
    /*取超级块*/
    if(!(sb = get_super(inode->dev)))
    {
        /*显示出错信息以供调试*/
        halts("try to read inode without dev.\n");
    }
    
    /*计算节点在磁盘上的位置*/
    block = 2 + sb->imap_blocks + sb->dmap_blocks 
        + ((inode->inode_num - 1) / INODES_PER_BLOCK);
    /*读取该位置的磁盘块*/
    if(!(mbuffer = bread(inode->dev,block)))
    {
        halts("can not read block.");
    }
    
    *(struct_inode *)inode = 
        ((struct_inode *)mbuffer->data)[(inode->inode_num - 1) % INODES_PER_BLOCK];

    /*最后，释放缓冲块*/
    brelease(mbuffer);
    unlock_inode(inode);
}

/*将内存中的指定inode写入设备*/
private void write_inode(struct_mem_inode *inode)
{
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    int block;
    
    /*锁 i节点，因为下面要使用它*/
    lock_inode(inode);
    
    /*如果节点没被修改过，或者设备号为0，则不写*/
    if((!inode->dirty) || (!inode->dev))
    {
        unlock_inode(inode);
        return;
    }
    
    /*获取设备的超级块*/
    if(!(sb = get_super(inode->dev)))
    {
        halts("write inode with no dev.");
    }
    
    /*计算节点在磁盘上的位置*/
    block = 2 + sb->imap_blocks + sb->dmap_blocks 
        + ((inode->inode_num - 1) / INODES_PER_BLOCK);
    /*读取该位置的磁盘块*/
    if(!(mbuffer = bread(inode->dev,block)))
    {
        halts("can not read block.");
    }
    
    ((struct_inode *)mbuffer->data)[(inode->inode_num - 1) % INODES_PER_BLOCK] 
        = *(struct_inode *)inode;
        
    /*置缓冲已修改标志，说明修改过了，这样会写入磁盘*/
    mbuffer->dirty = TRUE;
    /*置inode未修改，因为其内容已写入磁盘了*/
    inode->dirty = FALSE;
    /*释放缓冲*/
    brelease(mbuffer);
    unlock_inode(inode);
}
