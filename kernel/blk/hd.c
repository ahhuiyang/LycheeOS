/***************************************************************************
 *
 *  hd.c-硬盘底层操作函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 /*
 *  硬盘操作的一般步骤：
 *          1、检测寄存器空闲状态，CPU通过读状态寄存器，若位为0，表示控制器空闲，
 *          若在规定时间内控制器一直处于忙状态，则超时出错；
 *          2、检测驱动器是否就绪，CPU通过状态寄存器位6是否为1来看是否就绪，若就
 *          绪可输出参数和命令；
 *          3、输出命令块：按顺序输出分别向对应端口输出参数和命令。
 *          4、处理器等待中断产生：命令执行后，由硬盘控制器产生中断请求信号或置控
 *          器状态为空闲，表明操作结束或表示请求扇区传输。
 *          5、检测操作结果：CPU再次读取主状态寄存，若位0等于0则表示命令成功执行，
 *          否则失败，若失败可进一步查询错误寄存器取错误码。
 */
 
#include "kernel.h"
#include "hd.h"
#include "interrupt.h"
#include "type.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#include "oslib.h"
#include "blk.h"
#include "fs.h"

#define MAX_ERRORS  7

/*硬盘中断要调用的函数，读、写或复位指令都会重设此指针*/
private int_handler do_hd;

private int nr_hd;

/*硬盘和分区*/
public struct_hd hd[2] = {{0,0,0,0,0,0},{0,0,0,0,0,0}};
/*每个硬盘有4个分区，在下面的数组中，0和5代表整个硬盘*/
public struct_hd_partition hd_partition[10];

/*硬盘中断处理函数*/
public void irq_hd_handler()
{
    /*调用指定的中断处理函数*/
    (*do_hd)();
}

/*硬盘初始化，启用中断*/
void hd_init(void *bios_hd_data)
{
    int i,j;
    struct_mem_buffer *mbuffer;
    
    /*
    *   取硬盘数据，这些数据的指针在BIOS中断向量0x41和0x46两个地方
    *   第一个硬盘在0x41，第二个硬盘在0x46
    */
    for(i = 0 ; i < 2 ; i ++)
    {
        hd[i].cylinder = *(u16 *)(bios_hd_data);
        hd[i].head = *(u8 *)(bios_hd_data + 2);
        hd[i].precomp = *(u16 *)(bios_hd_data + 5);
        hd[i].ctl = *(u8 *)(bios_hd_data + 8);
        hd[i].land_zone = *(u16 *)(bios_hd_data + 0x0C);
        hd[i].sectors_per_track = *(u8 *)(bios_hd_data + 0x0E);
        
        bios_hd_data += 16;
    }
    
    /*判断系统中安装有多少块硬盘*/
    nr_hd = (hd[i].cylinder)?2:1;
    
    /*在hd_partition结构里，有两项代表硬盘*/
    for(i = 0 ; i < nr_hd ; i ++)
    {
        hd_partition[i * 5].start_sector = 0;   /*对于硬盘来说，起始扇区为0*/
        hd_partition[i * 5].nr_sectors = hd[i].head * hd[i].cylinder
            * hd[i].sectors_per_track;
    }
    
    /*
    *   下面要读分区了，如果磁盘分区表有效，则0x1fe(第一扇区最后两字节)处的字符是"55AA"
    *   分区表在0x1BE处开始，有四个
    */
    for(i = 0 ; i < nr_hd ; i ++)
    {
        /*读取第一个盘块的内容，盘块0*/
        if(!(mbuffer = bread(0x300 + i * 5,0)))
        {
            printfs("Unable to read partition table of driver %d\n",i);
            halt();
        }
        
        /*如果分区表不合法，则打印并停机*/
        if((unsigned char)mbuffer->data[510] != 0x55 
            || (unsigned char)mbuffer->data[511] != 0xAA)
            {
                printfs("Bad partition table on driver %d\n",i);
                halt();
            }
            
        struct_partition *partition = 0x1BE + (void *)mbuffer->data;
            /*取得数据*/
        for(j = 1 ; j < 5 ; j ++)
        {
            hd_partition[i * 5 + j].start_sector = partition->phy_start_sector;
            hd_partition[i * 5 + j].nr_sectors = partition->nr_sectors;
        }
        
        /*释放高速缓冲*/
        brelease(mbuffer);
    }
    
    printfs("Partition table read successful.\n");
    
    blk_dev[MAJOR_HD].do_request_fn = do_hd_request;
    set_irq_handler(IRQ_HD,irq_hd_handler);
    enable_irq(IRQ_CASCADE);
    enable_irq(IRQ_HD);
}

/*
*   判断并循环等待硬盘驱动器就绪
*/
private int is_hd_ready()
{
    int nRetries = 10000;
    
    while(--nRetries && ((in_byte(HD_STATE_REG) & (HD_STATE_READY | HD_STATE_BUSY)) != (HD_STATE_READY)));
    
    return nRetries;
}

/*检测硬盘执行命令后的状态，如果成功*/
private int is_hd_error()
{
    int state = in_byte(HD_STATE_REG);
    
    /*如果结果中Ready和Seek_end置位，则返回0，表示没有错误*/
    if(state & (HD_STATE_SEEK_END | HD_STATE_DRIVER_ERR | HD_STATE_READY
        | HD_STATE_BUSY | HD_STATE_ERROR | HD_STATE_ECC) == (HD_STATE_READY | HD_STATE_SEEK_END))
    {
        return 0;
    }
    
    /*如果出错则读出出错状态寄存器*/
    if(state & HD_STATE_ERROR)state = in_byte(HD_ERROR_REG);
    return 1;
}

/*等待硬盘达到某个状态，循环指定次数，若成功，返回值大于0，否则为0*/
private int waitfor_hd(int state)
{
    int n = 10000;
    
    while(--n && (in_byte(HD_STATE_REG) & state) != state);
    
    return n;
}

/*硬盘驱动器是否忙*/
private int is_hd_busy()
{
    int n = 10000;
    
    while(--n && (in_byte(HD_STATE_REG) & (HD_STATE_READY | HD_STATE_BUSY)) != HD_STATE_READY);
    
    return (!n);
}

/*
*向驱动器发送读或写命令
*       driver:         0或1
*       nsectors:       要读写的扇区数
*       start_sector:   磁道上的起始扇区号，从1开始
*       head:           磁头号
*       cyl:            柱面号
*       cmd:            发送给硬盘的命令
*       pfn_int:        命令执行结束后调用的函数
*/
private void hd_cmd_out(unsigned int driver,unsigned nsectors,
    unsigned start_sector,unsigned int head,unsigned int cyl,
    unsigned int cmd,void (* pfn_int)(void))
{
    /*难合法性*/
    if(driver > 1)
    {
        printfs("driver %d spcified does not exist.\n",driver);
        halt();
    }
    
    if(!is_hd_ready())
    {
        printfs("hd controller is not ready.\n");
        halt();
    }
    
    do_hd = pfn_int;
    
    /*依次输入各参数*/
    out_byte(hd[driver].ctl,HD_CTL_REG);
    out_byte(hd[driver].precomp >> 2,HD_PRECOMP_REG);
    out_byte(nsectors,HD_NSECTOR_REG);
    out_byte(start_sector,HD_SECTOR_REG);
    out_byte(cyl & 0xFF,HD_LCYL_REG);
    out_byte(cyl >> 8,HD_HCYL_REG);
    out_byte(0xA0 | (driver << 4) | head,HD_CURRENT_REG);
    out_byte(cmd,HD_CMD_REG);
    
    /*在中断中将对错误进行处理*/
}

/*这段代码我还不确定它的功能*/
private reset_controller()
{
    int i;
    
    /*向控制器发出复位命令*/
    out_byte(4,HD_CMD_REG);
    /*循环等待控制器复位*/
    for(i = 0 ; i <= 100 ; i ++)nop();
    /*再发送正常的控制字节*/
    out_byte(hd[0].ctl,HD_CMD_REG);
    /*如果磁盘仍忙，则显示出错信息*/
    if(is_hd_busy())
    {
        printfs("hd controller is still busy.\n");
    }
}

/*复位硬盘*/
private void reset_hd(int driver)
{
    if(driver > 1)return;
    
    reset_controller();
    hd_cmd_out(driver,hd[driver].sectors_per_track,
        hd[driver].sectors_per_track,hd[driver].head-1,
        hd[driver].cylinder,HD_CMD_SPECIFY,NULL);
}

/*发生意外中断时要调用的函数*/
void int_unexpected_hd()
{
    printfs("Unexpected hd interrupt.\n");
}

/*读写硬盘失败时要调用的函数*/
private void failed_rw()
{
    if(++blk_dev[MAJOR_HD].current_request->nr_errors >= MAX_ERRORS)
        end_request(MAJOR_HD,0);    /*硬盘，数据缓冲无效*/
}

private void int_hd_read()
{
    /*若读取出错*/
    if(is_hd_error())
    {
        failed_rw();        /*失败次数加1*/
        do_hd_request();    /*重新读取*/
        return;
    }
    
    /*读取成功，读数据，并将数据拷到指定内存缓冲区*/
    port_read(HD_DATA_REG,blk_dev[MAJOR_HD].current_request->buffer,256);
    blk_dev[MAJOR_HD].current_request->nr_errors = 0;
    blk_dev[MAJOR_HD].current_request->buffer += 512;
    blk_dev[MAJOR_HD].current_request->start_sector ++;
    if(--blk_dev[MAJOR_HD].current_request->nr_sectors)
    {
        do_hd = int_hd_read;
        return;
    }
    end_request(MAJOR_HD,DATA_YES);
    do_hd_request();
}

/*写操作中断调用的函数*/
private void int_hd_write()
{
    if(is_hd_error())
    {
        failed_rw();
        do_hd_request();
        return;
    }
    
    blk_dev[MAJOR_HD].current_request->nr_errors = 0;
    blk_dev[MAJOR_HD].current_request->buffer += 512;
    blk_dev[MAJOR_HD].current_request->start_sector ++;
    if(--blk_dev[MAJOR_HD].current_request->nr_sectors)
    {
        do_hd = int_hd_write;
        port_write(HD_DATA_REG,blk_dev[MAJOR_HD].current_request->buffer,256);
        return;
    }
    end_request(MAJOR_HD,DATA_YES);
    do_hd_request();
}

/*重新校正硬盘时发生中断，将会调用些函数*/
private void int_reset()
{
    if(is_hd_error())
    {
        failed_rw();
    }
    
    /*执行当前读写请求*/
    do_hd_request();
}

/*处理请求项*/
void do_hd_request()
{
    int dev,block;
    int head,sector,cylinder,nsectors;
    int i;
    
    /*如果当前请求项不合法，则退出*/
    if(blk_dev[MAJOR_HD].current_request->dev == -1)return;
    /*请求项的设备号是完整 的设备号，现在分离出子设备号*/
    dev = MINOR(blk_dev[MAJOR_HD].current_request->dev);
    /*相对于当前分区的起始扇区*/
    block = blk_dev[MAJOR_HD].current_request->start_sector;
    /*检测设备号和起始扇区的合法性*/
    if(dev > (5 * nr_hd) || block + 2 > blk_dev[MAJOR_HD].current_request->nr_sectors)
    {
        end_request(MAJOR_HD,0);
    }
    /*将相对扇区号转化为绝对扇区号，方法是加上当前分区的起始扇区号，这个扇区号是相对整个硬盘的*/
    block += hd_partition[dev].start_sector;
    /*现在从dev获取驱动器号*/
    dev /= 5;
    /*求出磁道号，磁头号，扇区*/
    /*扇区号从1开始，所以要加1*/
    sector = block % hd[dev].sectors_per_track + 1;
    /*算出总磁道*/
    block /= hd[dev].sectors_per_track;
    /*所在柱面*/
    cylinder = block / hd[dev].head;
    /*所在磁头*/
    head = block % hd[dev].head;
    /*读写扇区总数*/
    nsectors = blk_dev[MAJOR_HD].current_request->nr_sectors;
    
    /*下面分读和写区别对待*/
    if(blk_dev[MAJOR_HD].current_request->cmd == WRITE)
    {
        /*发送写命令*/
        hd_cmd_out(dev,nsectors,sector,head,cylinder,HD_CMD_WRITE,int_hd_write);
        /*等待硬盘接受请求*/
        for(i = 0 ; i < 10000 ; i ++)
        {
            if(in_byte(HD_STATE_REG) & HD_STATE_DRQ)break;
        }
        
        if(i == 10000)
        {
            failed_rw();
        }
        
        /*可以写数据了*/
        port_write(HD_DATA_REG,blk_dev[MAJOR_HD].current_request->buffer,256);
    }
    else if(blk_dev[MAJOR_HD].current_request->cmd == READ)
    {
        hd_cmd_out(dev,nsectors,sector,head,cylinder,HD_CMD_READ,int_hd_read);
    }
    else
    {
        printfs("unknown hd command.");
        halt();
    }
}
