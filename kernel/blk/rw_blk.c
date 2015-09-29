/***************************************************************************
 *
 *  rw_block.c-块设备与系统其它模块的读写接口
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "blk.h"
#include "memory.h"
#include "fs.h"
#include "hd.h"
#include "type.h"

private struct_process *proc_wait_for_request = NULL;

/*按柱面号排序,dev只能是0－7之间的值*/
private int in_order(struct_request *sr1,struct_request *sr2)
{
    /*读比写优先*/
    if(sr1->cmd < sr2->cmd)
    {
        return 1;
    }
    
    /*设备号小的优先*/
    if(sr1->cmd == sr2->cmd && (sr1->dev < sr2->dev))
    {
        return 1;
    }
    
    /*扇区号小的优先*/
    if(sr1->cmd == sr2->cmd && sr1->dev == sr2->dev 
        && sr1->start_sector < sr2->start_sector)
        {
        return 1;
        }
        
    return 0;
}
 
/*将请求项添加到指定的请求队列中*/
private void add_request(struct_blk_dev *bdev,struct_request *request)
{
    struct_request *tmp;
    
    /*如果当前设备请求队列 中没有当前请求，则说明队列中没有请求，则直接执行请求*/
    if(!bdev->current_request)
    {
        bdev->current_request = request;
        (bdev->do_request_fn)();
        return;
    }
    
    /*否则，使用电梯调度算法将当前请求项插入到队列中*/
    tmp = bdev->current_request;
    for( ; tmp->next ; tmp = tmp->next)
    {
        if((in_order(tmp,request) && in_order(request,tmp->next))
            || (in_order(request,tmp) && in_order(tmp->next,request)))
        {
            break;
        }
    }
    
    request->next = tmp->next;
    tmp->next = request;
}

public void rw_blk(int rw,struct_mem_buffer *mbuffer)
{
    unsigned int i;
    unsigned int major = MAJOR(mbuffer->dev);
    struct_request *req = NULL;
    
    if(major > 7 || (!blk_dev[major].do_request_fn))
    {
        printfs("the device %d is not available.\n",major);
        return;
    }
    
    lock_mem_buffer(mbuffer);
    
    /*如果命令是写但数据没改过，或命令是读但数据有效，则返回，不需要操作*/
    if((rw == READ && mbuffer->uptodate) || (rw == WRITE && (!mbuffer->dirty)))
    {
        unlock_mem_buffer(mbuffer);
        return;
    }
    
    while(1)
    {
        /*将当前请求项插入请求项队列，如果请求项没有空间，则等待*/
        for(i = 0 ; i < NR_REQUEST ; i ++)
     {
            if(blk_request[i].dev == -1)break;
      }
    
        /*如果没有搜索到空闲的请求项，则睡眠当前进程*/
        if(i < NR_REQUEST)
        {
            req = &blk_request[i];
            break;
        }
        else
        {
            sleep_on(&proc_wait_for_request);
            continue;
        }
    }
    
    /*到这里，已经找到了空闲的请求项*/
    req->dev = mbuffer->dev;    /*完整的设备号*/
    req->cmd = rw;
    req->nr_errors = 0;
    req->start_sector = mbuffer->block<<2;
    req->nr_sectors = BLOCK_SIZE / SECTOR_SIZE; /*每次只读写一个缓冲的数据，即1KB*/
    req->buffer = mbuffer->data;
    req->proc_wait = NULL;
    req->mbuffer = mbuffer;
    req->next = NULL;
    
    add_request(&blk_dev[major],req);
}

/*初始化块设备，blk_dev的初始化在对应的块设备中*/
public void blk_dev_init()
{
    int i;
    
    for(i = 0 ; i <= NR_REQUEST ; i ++)
    {
        blk_request[i].dev = -1;
    }
}
