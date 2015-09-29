/***************************************************************************
 *
 *  kernel.c-跟gdt、ldt、idt有关的操作
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
/*descriptor和gate，还有sys_idt*/
#include "kernel.h"
#include "type.h"
#include "interrupt.h"
#include "io.h"
#include "string.h"
#include "memory.h"

/*系统启动以来，时钟滴答数*/
public unsigned int sys_ticks;
/*系统从1970-1-1  0:0:0的秒数*/
public unsigned int sys_startup_time;

/*系统内存区*/
public unsigned int sys_memory_start;
public unsigned int sys_memory_end;
public unsigned int buffer_memory_start;
public unsigned int buffer_memory_end;
public unsigned int main_memory_start;
public unsigned int main_memory_end;

/*硬盘信息*/
public void *sys_hd_info;

/*系统根设备目录*/
public int sys_root_dev;

/*当前运行的进程*/
public struct_process *current;

/*
 * public and private are defined in type.h
 */
 
private inline void set_gate_descriptor(int vector,u32 offset,u16 selector,
    u8 pcount,u8 attribute,u8 dpl)
{
    struct_gate *pGate = (struct_gate *)&sys_idt[vector];
    pGate->offset0_15 = offset & 0xff;
    pGate->offset16_31 = (offset >> 16) & 0xff;
    pGate->pcount = pcount;
    pGate->selector = selector;
    pGate->attribute = attribute | (dpl & 0x60);
}
 
 /*
 *set_gate:
 *  vector-中断向量号
 *  handler-中断处理程序，原型定义在type.h
 *  gate_type-门类型
 *  dpl-门特权级
 */
private inline void set_gate(int vector,int_handler handler,u8 gate_type,u8 dpl)
{
    u32 offset = (u32)handler;
    set_gate_descriptor(vector,offset,KERNEL_CS,0,gate_type,dpl);
}

/*中断门，0特权级*/
public inline void set_interrupt_gate(int vector,int_handler handler)
{
    set_gate(vector,handler,GATE_INT,DPL_0);
}

/*陷井门，0特权级*/
public inline void set_trap_gate(int vector,int_handler handler)
{
    set_gate(vector,handler,GATE_TRAP,DPL_0);
}

/*系统调用，3特权级*/
public inline void set_system_gate(int vector,int_handler handler)
{
    set_gate(vector,handler,GATE_TRAP,DPL_3);
}

/*如果系统出现错误，则停止系统，调用此函数*/
public void halt(void)
{
    while(1);
}

/*带字符串显示的死机函数*/
public inline void halts(char *str)
{
    printfs(str);
    halt();
}

/*设备段描述符*/
public inline void set_descriptor(struct_descriptor *sd,u16 limit0_15,
    u16 base0_15,u8 base16_23,
    u8 attribute,u8 segattr,u8 base24_31)
{
    sd->limit0_15 = limit0_15;
    sd->base0_15 = base0_15;
    sd->base16_23 = base16_23;
    sd->attribute = attribute;
    sd->segattr = segattr;
    sd->base24_31 = base24_31;
}

/*设置LDT的描述符*/
public inline void set_ldt_desc(struct_descriptor *sd,u16 limit,u32 base)
{
    set_descriptor(sd,limit,base & 0xFFFF,(base >> 16) & 0xFF,
        DESC_LDT,0,(base >> 24) & 0xFF);
}

/*设备TSS描述符*/
public inline void set_tss_desc(struct_descriptor *sd,u16 limit,u32 base)
{
    set_descriptor(sd,limit,base & 0xFFFF,(base >> 16) & 0xFF,
        DESC_TSS,0,(base >> 24) & 0xFF);
}

/*取得段限长*/
public inline u32 get_seg_limit(struct_descriptor *sd)
{
    u32 ret;
    
    if((sd->segattr & 0x80))
    {
        ret = (sd->limit0_15 + (((sd->segattr) & 0x0F) << 16)) * 4 * KB;
    }
    else
    {
        ret = sd->limit0_15 + (((sd->segattr) & 0x0F) << 16);
    }
    
    return ret;
}

/*取得段基址*/
public inline int get_seg_base(struct_descriptor *sd)
{
    return (sd->base0_15 + (sd->base16_23 << 16) + (sd->base24_31 << 24));
}

/*设置段基址*/
public inline void set_seg_base(struct_descriptor *sd,u32 base)
{
    sd->base0_15 = base & 0xFFFF;
    sd->base16_23 = (base >> 16) & 0xFF;
    sd->base24_31 = (base >> 24) & 0xFF;
}
