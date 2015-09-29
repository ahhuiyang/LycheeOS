/***************************************************************************
 *
 *  kernel.h-内核全部变量定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 /*
 *  内核全局变量，比如 GDTR、LDTR、IDT的值
 */
 
#ifndef _KERNEL_H
#define _KERNEL_H

#include "type.h"
#include "time.h"

#define GDT_SIZE     5
#define IDT_SIZE     256

/*内核代码段和数据段选择符*/
#define KERNEL_CS       0x08    /*第二个描述符的段选择符(selector for descriptor)*/
#define KERNEL_DS       0x10    /*第三个描述符的段选择符*/
#define TASK_LDT        0x18    /*第四个描述符的段选择符*/
#define TASK_TSS        0x20    /*第五个描述符的段选择符*/

/*所有任务的代码段、数据段和堆栈段*/
#define TASK_CS         0x0F
#define TASK_DS         0x17

/*gdt表中的索引*/
#define INDEX_GDT_NULL  0
#define INDEX_GDT_CS    1
#define INDEX_GDT_DS    2
#define INDEX_GDT_LDT   3
#define INDEX_GDT_TSS   4
#define INDEX_USER_CS   1
#define INDEX_USER_DS   2

/*特权级*/
#define PRIVILEGE_KERNEL    0
#define PRIVILEGE_USER      3

/*段或门属性*/
#define DPL_0       0x00
#define DPL_1       0x20
#define DPL_2       0x40
#define DPL_3       0x60

#define GATE_TASK   0x85       /*任务门*/
#define GATE_CALL   0x8C       /*调用门*/
#define GATE_INT    0x8E       /*中断门*/
#define GATE_TRAP   0x8F       /*陷井门*/

#define DESC_LDT    0x82       /*LDT*/
#define DESC_TSS    0x89       /*TSS*/

/*每个进程64MB的内存*/
#define PROCESS_MEM_SPACE   0x4000000

/*系统中最多能运行的进程*/
#define NR_PROCESS      16
#define NR_MAX_FILES    8

/*segment descriptor,8 bytes total*/
/*define in detail,12 bytes total*/
typedef struct _struct_descriptor_detail
{
    unsigned int limit0_15:16;      /*段长度低16位*/
    unsigned int base0_23:24;       /*段基址的低24位*/
    unsigned int type:4;            /*段类型，与s一同作用*/
    unsigned int s:1;               /*描述符类型，1-代码或数据段，0-系统段或门*/
    unsigned int dpl:2;             /*段的特权级*/
    unsigned int p:1;               /*0-段不在内存，1-段在内存*/
    unsigned int limit16_19:4;      /*段长度的最高4位*/
    unsigned int avl:1;             /*avaiable，供系统软件使用*/
    unsigned int reserved:1;        /*保留，必须为0*/
    unsigned int d_b:1;             /*default operation size,0-16,1-32*/
    unsigned int g:1;               /*分配粒度，0-byte，1-4KB*/
    unsigned int base24_31:8;       /*基地址高8位*/
}struct_descriptor_detail;

/*normal define,8 bytes*/
typedef struct _struct_descriptor
{
    u16 limit0_15;
    u16 base0_15;
    u8 base16_23;
    u8 attribute;
    u8 segattr;
    u8 base24_31;
}struct_descriptor;

/*gate descriptor,8 bytes total*/
/*define in detail,9 bytes*/
typedef struct _struct_gate_detail
{
    unsigned int offset0_15:16;     /*偏移低16位*/
    unsigned int selector:16;       /*段选择符*/
    unsigned int paramcount:5;      /*参数个数*/
    unsigned int reserved:3;        /*保留，必须为0*/
    unsigned int type:4;            /*段类型，与s一起使用*/
    unsigned int s:1;               /*描述符类型，0-代码或数据段，1-系统段或门*/
    unsigned int dpl:2;             /*段的特权级*/
    unsigned int p:1;               /*0-段不在内存，1-段在内存*/
    unsigned offset16_31:16;        /*偏移高16位*/
}struct_gate_detail;

/*normal define,8 bytes*/
typedef struct _struct_gate
{
    u16 offset0_15;
    u16 selector;
    u8 pcount;
    u8 attribute;
    u16 offset16_31;
}struct_gate;

typedef struct _struct_tss struct_tss;
typedef struct _struct_process struct_process;

/*gdt,idt,declare in boot/rmtopm.asm*/
extern struct_descriptor sys_gdt[GDT_SIZE];
extern unsigned char sys_gdt_48[6];
extern struct_gate sys_idt[IDT_SIZE];
extern unsigned char sys_idt_48[6];

/*defined in kernel.c*/
extern unsigned int sys_ticks;
extern unsigned int sys_startup_time;

/*在tty.c中的一个常量，表示当前的tty号*/
extern unsigned int nr_current_tty;

/*页目录和页表*/
extern unsigned int sys_pgdir;
extern unsigned int sys_pgtbl;

/*系统堆栈*/
extern int sys_stack[1024];

public extern struct_process *process[NR_PROCESS];
/*当前运行的进程*/
public extern struct_process *current;

/*当前时间，以秒计*/
#define CURRENT_TIME (sys_startup_time + sys_ticks/HZ)

/*内存区*/
public extern unsigned int sys_memory_start;
public extern unsigned int sys_memory_end;
public extern unsigned int buffer_memory_start;
public extern unsigned int buffer_memory_end;
public extern unsigned int main_memory_start;
public extern unsigned int main_memory_end;

/*根设备*/
public extern int sys_root_dev;

/*硬盘信息，定义在kernel.c中*/
public extern void *sys_hd_info;

/*kernel.asm*/
public void reload_cr3(void);
public void load_ldt(u16 ldt);
public void load_tr(u16 tss);

/*操作描述符和门的函数原型，下面三个函数在kernel.c中声明并实现*/
/*中断门，0特权级*/
public inline void set_interrupt_gate(int vector,int_handler handler);
/*陷井门，0特权级*/
public inline void set_trap_gate(int vector,int_handler handler);
/*系统调用，3特权级*/
public inline void set_system_gate(int vector,int_handler handler);
/*下面的三个函数在i8259.c中声明并实现*/
/*开启irq中断*/
public void enable_irq(int irq);
/*开闭irq中断*/
public void disable_irq(int irq);
/*设置irq中断处理程序*/
public void set_irq_handler(u8 irq,void *handler);
/*中断系统*/
public void halt(void);
public inline void halts(char *str);
public inline void set_descriptor(struct_descriptor *sd,u16 limit0_15,
    u16 base0_15,u8 base16_23,
    u8 attribute,u8 segattr,u8 base24_31);

#endif
