/***************************************************************************
 *
 *  time.h-操作系统时间相关的数据结构和常量
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _TIME_H
#define _TIME_H

#include "type.h"

#define SECOND  1
#define MINUTE  (60 * SECOND)
#define HOUR    (60 * MINUTE)
#define DAY     (24 * HOUR)
#define YEAR    (365 * DAY)     /*注意这不是闰年的年*/

/*一个闰年的12个月*/
/*月份从零开始纯粹是为了对应此数组*/
public unsigned extern int days_of_month[12];

/*时钟的输入频率*/
#define TIMER_FREQUENCY 1193182L
#define HZ              100         /*100HZ = 10ms*/

/*8253模式控制寄存器*/
#define TIMER_CTL_REG       0x43
#define TIMER_COUNTER0      0x41    /*输出到IRQ0，以便每隔一次让系统产生一次时钟中断*/
#define TIMER_COUNTER1      0x42    /*通常被设为18，以便 大约15us做一次RAM刷新*/
#define TIMER_COUNTER2      0x43    /*连接PC喇叭*/

/*要初始化时钟，首先要向0x43发送0x34*/
#define TIMER_RATE_GENERATOR    0x34

/*时间结构体*/
typedef struct _struct_time
{
    unsigned int second;
    unsigned int minute;
    unsigned int hour;
    unsigned int day;
    unsigned int month;         /*从0开始*/
    unsigned int year;
    
    unsigned int day_of_week;   /*一星期的某天，[0-6]对应星期天－星期一*/
    unsigned int day_of_year;   /*一年中的某天，[0-365]*/
}struct_time;

#endif
