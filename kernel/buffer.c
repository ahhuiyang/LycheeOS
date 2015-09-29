/***************************************************************************
 *
 *  buffer.c-磁盘高速缓冲区的管理
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "kernel.h"
#include "string.h"
#include "memory.h"
#include "const.h"
#include "type.h"
#include "blk.h"

public struct_mem_buffer *mem_buffers;
private struct_process *proc_wait_buffer;
int nr_buffers = 0;

public void lock_mem_buffer(struct_mem_buffer *mbuffer)
{
    disable_int();
    
    /*如果已经上锁了，则当前进程等待*/
    while(mbuffer->locked)
    {
        sleep_on(&mbuffer->proc_wait);
    }
    
    mbuffer->locked = TRUE;
    
    enable_int();
}

public void unlock_mem_buffer(struct_mem_buffer *mbuffer)
{
    /*如果内存缓冲没有上锁，则显示警告信息以供调试*/
    if(!mbuffer->locked)
    {
        printfs("try to unlock mem buffer that is not locked.\n");
    }
    mbuffer->locked = FALSE;
    wake_up(&mbuffer->proc_wait);
}

/*等待指定的缓冲区，注意并没有更改locked属性*/
public  void wait_on_buffer(struct_mem_buffer *mbuffer)
{
    while(mbuffer->locked)
        sleep_on(&mbuffer->proc_wait);
}

/*同步指定设备和高速缓冲之间的数据，即把数据写入设备*/
public void sync_dev(int dev)
{
    int i;
    
    for(i = 0 ; i < nr_buffers ; i ++)
    {
        if(mem_buffers[i].dev == dev)
        {
            /*首先等待缓冲区*/
            wait_on_buffer(&mem_buffers[i]);
            /*进程睡眠过，再判断一次设备，如果数据修改过，则写入缓冲区*/
            if(mem_buffers[i].dev == dev && mem_buffers[i].dirty)
            {
                ll_rw(WRITE,&mem_buffers[i]);
            }
        }
    }
}

/*使指定设备在高速缓冲的数据无效*/
public void invalid_dev_buffers(int dev)
{
    int i;
    
    for(i = 0 ; i < nr_buffers ; i ++)
    {
        if(mem_buffers[i].dev == dev)
        {
            /*首先等待缓冲区*/
            wait_on_buffer(&mem_buffers[i]);
            /*清除dirty，所以不会写回，清除uptodate，所以会重读*/
            if(mem_buffers[i].dev == dev)
            {
                mem_buffers[i].dirty = 0;
                mem_buffers[i].uptodate = 0;
            }
        }
    }
}

/*查找指定设备号和指定块号的高速缓冲块是否存在，若存在则返回缓冲块结构*/
public struct_mem_buffer *find_buffer(int dev,int block)
{
    int i;
    
    for(i = 0 ; i < nr_buffers ; i ++)
    {
        if(mem_buffers[i].dev == dev && mem_buffers[i].block == block)
        {
            mem_buffers[i].count ++;
            return &mem_buffers[i];
        }
    }
    
    return NULL;
}

/*获取空闲的缓冲块，如果数据块已被读进缓冲，则直接返回*/
public struct_mem_buffer *get_buffer(int dev,int block)
{
    int i;
    struct_mem_buffer *mbuffer;
    
    /*如果内存缓冲中能找到，则直接返回*/
    if(mbuffer = find_buffer(dev,block))
    {
        return mbuffer;
    }
    
    /*直到找到缓冲为止*/
    while(1)
    {
        for(i = 0 ; i < nr_buffers ; i ++)
        {
            if(!mem_buffers[i].count)
            {
                break;
            }
        }
        
        /*如果没找到*/
        if(i == nr_buffers)
        {
            sleep_on(&proc_wait_buffer);
            continue;
        }
        
        /*如果已被上锁，等待解锁*/
        wait_on_buffer(&mem_buffers[i]);
        /*如果引用计数又大于0的话，则重复上述过程*/
        if(mem_buffers[i].count)
        {
            continue;
        }
        
        while(mem_buffers[i].dirty)
        {
            rw_blk(WRITE,&mem_buffers[i]);
        }
        
        /*继续等待*/
        wait_on_buffer(&mem_buffers[i]);
        if(mem_buffers[i].count)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    
    mem_buffers[i].count = 1;
    mem_buffers[i].dirty = 0;
    mem_buffers[i].uptodate = 0;
    mem_buffers[i].dev = dev;
    mem_buffers[i].block = block;
    
    return &mem_buffers[i];
}

/*释放指定设备指定块号的缓冲块*/
public void brelease(struct_mem_buffer *mbuffer)
{
    /*参数合法性检查*/
    if(!mbuffer)
    {
        return;
    }
    
    /*等待缓冲区*/
    wait_on_buffer(mbuffer);
    /*如果引用计数零，则说明内核有错*/
    if(!mbuffer->count)
    {
        printfs("try to release free mem buffer.\n");
        halt();
    }
    
    mbuffer->count --;
    wake_up(&mbuffer->proc_wait);
}

/*从指定设备读指定块号的数据*/
public struct_mem_buffer *bread(int dev,int block)
{
    struct_mem_buffer *mbuffer;
    
    if(!(mbuffer = get_buffer(dev,block)))
    {
        printfs("can not get free buffer.\n");
        halt();
    }
    
    /*如果缓冲中的数据有效，则直接返回*/
    if(mbuffer->uptodate)
    {
        return mbuffer;
    }
    
    rw_blk(READ,mbuffer);
    
    wait_on_buffer(mbuffer);
    
    /*如果数据有效，则返回*/
    if(mbuffer->uptodate)
    {
        return mbuffer;
    }
    
    brelease(mbuffer);
    return NULL;
}

/*初始化*/
public void mem_buffer_init()
{
    mem_buffers = (struct_mem_buffer *)buffer_memory_start;
    
    struct_mem_buffer *mb = mem_buffers;
    void *temp;
    
    if(buffer_memory_end == 1<<20)
    {
        temp = (void *)(640 * 1024);
    }
    else
    {
        temp = (void *)buffer_memory_end;
    }
    
    /*下面开初始化缓冲区和缓冲结构*/
    while((temp -= BLOCK_SIZE) >= (void *)(mb + 1))
    {
        mb->data = (char *)temp;
        mb->block = 0;
        mb->dev = -1;
        mb->uptodate = 0;
        mb->dirty = 0;
        mb->count = 0;
        mb->locked = FALSE;
        mb->proc_wait = NULL;
        mb->prev = mb - 1;
        mb->next = mb + 1;
        mb ++;
        nr_buffers ++;
        
        /*如果temp到达1MB的位置，则跳过384KB大小，即0xA0000-0xFFFFF*/
        if(temp == (void *)(1 << 20))
        {
            temp = (void *)0xA0000;
        }
    }
    
    if(nr_buffers > 0)
    {
        mem_buffers[0].prev = mem_buffers + nr_buffers - 1;
    }
}
