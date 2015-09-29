/******************************************************************************************
*
*   interrupt.h-中断号常量定义
*       
*   Copyright (C) 2010 杨习辉
*
*******************************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

/*int*/
#define NR_INT_DIVIDE_ERROR             0
#define NR_INT_SINGLE_STEP              1
#define NR_INT_NMI                      2
#define NR_INT_INT3                     3
#define NR_INT_OVERFLOW                 4
#define NR_INT_BOUNDS_CHECK             5
#define NR_INT_INVALID_OPCODE           6
#define NR_INT_COPROCESSOR_NOT_AVL      7
#define NR_INT_DOUBLE_FAULT             8
#define NR_INT_COPROCESSOR_SEG_OVERRUN  9
#define NR_INT_INVALID_TSS              10
#define NR_INT_SEG_NOT_PRESENT          11
#define NR_INT_STACK_EXCEPTION          12
#define NR_INT_GENERAL_PROTECTION       13
#define NR_INT_PAGE_FAULT               14
#define NR_INT_COPROCESSOR_ERROR        16

/*hardware interrupts*/
#define NR_INT_IRQ0         0x20
#define IRQ_CLOCK           NR_INT_IRQ0 + 0
#define IRQ_KEYBOARD        NR_INT_IRQ0 + 1
#define IRQ_CASCADE         NR_INT_IRQ0 + 2
#define IRQ_COM2            NR_INT_IRQ0 + 3
#define IRQ_COM1            NR_INT_IRQ0 + 4
#define IRQ_LPT2            NR_INT_IRQ0 + 5
#define IRQ_FLOPPY          NR_INT_IRQ0 + 6
#define IRQ_LPT1            NR_INT_IRQ0 + 7
#define IRQ_REAL_CLOCK      NR_INT_IRQ0 + 8
#define IRQ_HD              NR_INT_IRQ0 + 14
                     
#endif
