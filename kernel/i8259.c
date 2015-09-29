/***************************************************************************
 *
 *  i8259.c-操作8259芯片
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
#include "io.h"
#include "const.h"
#include "type.h"
#include "kernel.h"
 
#define NR_IRQS     16

/*外部函数，定义在i8259.asm中*/
extern void irq_0(void);
extern void irq_1(void);
extern void irq_2(void);
extern void irq_3(void);
extern void irq_4(void);
extern void irq_5(void);
extern void irq_6(void);
extern void irq_7(void);
extern void irq_8(void);
extern void irq_9(void);
extern void irq_10(void);
extern void irq_11(void);
extern void irq_12(void);
extern void irq_13(void);
extern void irq_14(void);
extern void irq_15(void);

/*
*
*   在这里设一个全局数组，是为了将IRQ0-15中断的安装达到这样的效果
*   比如在键盘设备驱动模块，先写出一个int_handler类型的函数
*   然后通过set_irq_handler()函数就可动态安装这个中断处理函数
*
*/
void *irq_handler_table[16];
 
public void init_8259A()
{
    /*Master,ICW1*/
    out_byte(I8259_MASTER_CTL,0x11);    /*icw4 needed,master*/
    /*Slave,ICW1*/
    out_byte(I8259_SLAVE_CTL,0x11);     /*icw4 needed,slave*/
    /*master,IRQ0-INT 0x20,ICW2*/
    out_byte(I8259_MASTER_MASK,INT_IRQ0);
    /*slave,IRQ8-INT 28h*/
    out_byte(I8259_SLAVE_MASK,INT_IRQ8);
    /*master,icw3*/
    out_byte(I8259_MASTER_MASK,0x4);     /*IRQ2级联从片，对应从8259*/
    /*slave,icw3*/
    out_byte(I8259_SLAVE_MASK,0x2);      /*从片连到主片上哪一个IRQ上，此处是2，即IRQ2*/
    /*icw4 for master and slave*/
    out_byte(I8259_MASTER_MASK,0x1);
    out_byte(I8259_MASTER_MASK,0x1);
    
    /*close all the interupt,use ocw1*/
    out_byte(I8259_MASTER_MASK,0xFF);
    out_byte(I8259_SLAVE_MASK,0xFF);
    
    /*init idt of irq0-15*/
    set_interrupt_gate(INT_IRQ0 + 0,irq_0);
    set_interrupt_gate(INT_IRQ0 + 1,irq_1);
    set_interrupt_gate(INT_IRQ0 + 2,irq_2);
    set_interrupt_gate(INT_IRQ0 + 3,irq_3);
    set_interrupt_gate(INT_IRQ0 + 4,irq_4);
    set_interrupt_gate(INT_IRQ0 + 5,irq_5);
    set_interrupt_gate(INT_IRQ0 + 6,irq_6);
    set_interrupt_gate(INT_IRQ0 + 7,irq_7);
    set_interrupt_gate(INT_IRQ0 + 8,irq_8);
    set_interrupt_gate(INT_IRQ0 + 9,irq_9);
    set_interrupt_gate(INT_IRQ0 + 10,irq_10);
    set_interrupt_gate(INT_IRQ0 + 11,irq_11);
    set_interrupt_gate(INT_IRQ0 + 12,irq_12);
    set_interrupt_gate(INT_IRQ0 + 13,irq_13);
    set_interrupt_gate(INT_IRQ0 + 14,irq_14);
    set_interrupt_gate(INT_IRQ0 + 15,irq_15);
}

public void enable_irq(int irq)
{
    unsigned char dl = (irq <= 7)?I8259_MASTER_MASK:I8259_SLAVE_MASK;
    unsigned char al = in_byte(dl);
    out_byte(al | (1 << (irq % 8)),dl);
}

public void disable_irq(int irq)
{
    unsigned char dl = (irq <= 7)?I8259_MASTER_MASK:I8259_SLAVE_MASK;
    unsigned char al = in_byte(dl);
    out_byte(al & (~(1 << (irq % 8))),dl);
}

public void set_irq_handler(u8 irq,void *handler)
{
    disable_irq(irq);
    irq_handler_table[irq] = handler;
}
