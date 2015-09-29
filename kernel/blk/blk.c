/***************************************************************************
 *
 *  blk.c-块设备与系统其它模块的接口
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "blk.h"
#include "type.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#include "buffer.h"

public struct_blk_dev blk_dev[NR_BLK_DEVICE];
public struct_request blk_request[NR_REQUEST]; 

public void end_request(int dev,int uptodate)
{
    if(blk_dev[dev].current_request->mbuffer)
    {
        blk_dev[dev].current_request->mbuffer->uptodate = uptodate;
        unlock_mem_buffer(blk_dev[dev].current_request->mbuffer);
    }
    
    /*如果数据无效，说明读取错误，显示出错信息以帮助调试*/
    if(!uptodate)
    {
        printfs("failed io on device %d",dev);
    }
    
    wake_up(blk_dev[dev].current_request->proc_wait);
    /*释放请求项*/
    blk_dev[dev].current_request->dev = -1;
    blk_dev[dev].current_request = blk_dev[dev].current_request->next;
}
