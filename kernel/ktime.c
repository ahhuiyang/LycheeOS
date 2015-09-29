/***************************************************************************
 *
 *  ktime.c-系统中跟时间有关的操作
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "kernel.h"
#include "type.h"
#include "io.h"
#include "time.h"

public unsigned int days_of_month[12] = {
    0x0,
    DAY * (31),
    DAY * (31 + 29),
    DAY * (31 + 29 + 31),
    DAY * (31 + 29 + 31 + 30),
    DAY * (31 + 29 + 31 + 30 + 31),
    DAY * (31 + 29 + 31 + 30 + 31 + 30),
    DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31),
    DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
    DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};

public unsigned int get_startup_time(struct_time *tm)
{
    unsigned int nRet = 0;
    unsigned int year;
    
    year = tm->year - 70;
    /*
    *
    *tm->year是两位的，会有2000年问题，因此在读取的时候会有如下代码
    *if(tm->year < 70)tm->year += 100;
    *
    */
    
    /*年 + 过去年的闰年数*/
    nRet = year * YEAR + (1 + (year -3) / 4) * DAY;
    nRet += days_of_month[tm->month];
    
    /*如果当前不是闰年而且大于等于2月，那么就要去掉一天的秒数了*/
    if((tm->month >= 2) && ((year + 2) % 4))
        nRet -= DAY;
    nRet += (tm->day - 1) * DAY;
    nRet += tm->hour * HOUR;
    nRet += tm->minute * MINUTE;
    nRet += tm->second;
    
    return nRet;
}

public void timer_init()
{
    /*初始化8253 PIT*/
    /*发送控制字节，要求设置计数值，每10ms发生一次时钟中断*/
    out_byte(TIMER_CTL_REG,TIMER_RATE_GENERATOR);
    /*设置计数值低8位*/
    out_byte(TIMER_COUNTER0,(TIMER_FREQUENCY / HZ) & 0xFF);
    /*高8位*/
    out_byte(TIMER_COUNTER0,((TIMER_FREQUENCY / HZ) >> 8) & 0xFF);
}
