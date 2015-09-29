/***************************************************************************
 *
 *  hd.h-硬盘相关的常量及结构定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _HD_H
#define _HD_H

/*ATA0设备上的主（Primary）硬盘控制器端口*/
#define HD_DATA_REG     0x1F0   /*读写，数据寄存器，扇区数据（读、写、格式化）*/
#define HD_ERROR_REG    0x1F1   /*只读，错误寄存器*/
#define HD_PRECOMP_REG  0x1F1   /*只写，写前预补偿寄存器*/
#define HD_NSECTOR_REG  0x1F2   /*读写，扇区数寄存器，扇区数（读、写、检验、格式化）*/
#define HD_SECTOR_REG   0x1F3   /*读写，扇区号寄存器，起始扇区（读、写，检验）*/
#define HD_LCYL_REG     0x1F4   /*读写，柱面号低字节（读、写、检验、格式化）*/
#define HD_HCYL_REG     0x1F5   /*读写，柱面号高字节（读、写、检验、格式化）*/
#define HD_CURRENT_REG  0x1F6   /*读写，驱动器／磁头寄存器（101dhhhh,d=驱动器号,h=磁头号）*/
#define HD_STATE_REG    0x1F7   /*只读，主状态寄存器*/
#define HD_CMD_REG      0x1F7   /*只写，命令寄存器*/
#define HD_CTL_REG      0x3F6   /*硬盘控制寄存器*/

/*状态寄存器中的状态位*/
#define HD_STATE_ERROR  0x01    /*命令执行错误*/
#define HD_STATE_INDEX  0x02    /*收到索引，当旋转收到索引标志时设置此位*/
#define HD_STATE_ECC    0x04    /*ECC校验*/
/*
*   当下面的位被置位时，表示驱动器已准备好在主机和
*   数据端口之间传输一个字或一个字节的数据
*/
#define HD_STATE_DRQ        0x08    /*数据请求服务*/
#define HD_STATE_SEEK_END   0x10    /*录道结束标志*/
#define HD_STATE_DRIVER_ERR 0x20    /*驱动器故障*/
#define HD_STATE_READY      0x40    /*驱动器已准备好，发生错误时该位不会改变，读取之后才能再次表示当前就绪状态*/
#define HD_STATE_BUSY       0x80    /*控制器忙碌*/

/*AT硬盘控制器命令*/
#define HD_CMD_RESET        0x10    /*驱动器重新较正位，把磁头移到0柱面上*/
#define HD_CMD_READ         0x20    /*读，以512B为单位，不加ECC，允许重试*/
#define HD_CMD_WRITE        0x30    /*写，以512B为单位，不加ECC，允许重试*/
#define HD_CMD_VERIFY       0x40    /*扇区校验，类似读操作，但不产生结果，只检测问题*/
#define HD_CMD_FORMAT       0x50    /*格式化磁道*/
#define HD_CMD_INIT         0x60    /*驱动器初始化*/
#define HD_CMD_SEEK         0x70    /*寻道操作*/
#define HD_CMD_DIAGNOSE     0x90    /*控制器诊断*/
#define HD_CMD_SPECIFY      0x91    /*建立驱动器参数命令*/ 

/*
*   硬盘分区表结构，这个结构是通用的，非常底层
*   在BIOS中断0x41和0x46处分别存放着这些数据
*           磁头范围：0-255
*           扇区号：    1-63
*           柱面号：    0-1023
*/
typedef struct _struct_partition
{
    unsigned char bootable;         /*80h=可引导，00h=不可引导，其它*/
    unsigned char start_head;       /*起始磁头号*/
    unsigned char start_sector;     /*起始扇区号（0-5位，柱面中），高两位为起始柱面号的8、9位*/
    unsigned char start_cyl;        /*起始柱面号的低8位*/
    unsigned char sys_id;           /*分区类型*/
    unsigned char end_head;         /*结束磁头号*/
    unsigned char end_sector;       /*结束扇区号（0-5位），高两位为结束柱面号的8-9位*/
    unsigned char end_cyl;          /*结束柱面号的低8位*/
    unsigned int phy_start_sector;  /*起始扇区号，从整个硬盘顺序，从0计起*/
    unsigned int nr_sectors;        /*扇区数*/
}struct_partition;

/*硬盘*/
typedef struct _struct_hd
{
    u16 head;              /*磁头数*/
    u16 cylinder;          /*柱面数*/
    u16 precomp;           /*写前预补偿*/
    u16 land_zone;         /*磁头着陆柱（停止）面号*/
    u8 sectors_per_track;  /*每磁道扇区数*/
    u8 ctl;                /*控制字节*/
}struct_hd;

/*真正有用的分区表数据项*/
typedef struct _struct_hd_partition
{
    unsigned int start_sector;      /*物理起始扇区号*/
    unsigned int nr_sectors;        /*扇区总数*/
}struct_hd_partition;

/*定义在hd.c*/
public extern struct_hd hd[2];
public extern struct_hd_partition hd_partition[10];

public extern void do_hd_request();

#endif
