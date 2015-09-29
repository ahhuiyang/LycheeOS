/******************************************************************************************
*
*   interrupt.c-int0-int16中断处理程序，接口在
*       interrupt.asm
*   Copyright (C) 2010 杨习辉
*
*******************************************************************************************/

#include "type.h"
#include "interrupt.h"

/**/

/*
*函数原型的声明，这些函数的实现在kernel/interrupt.asm
*page_fault函数在mm/pf.asm中实现
*/
void divide_error(void);
void single_step(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds_check(void);
void invalid_opcode(void);
void coprocessor_not_avl(void);
void double_fault(void);
void coprocessor_seg_overrun(void);
void invalid_tss(void);
void seg_not_present(void);
void stack_exception(void);
void general_protection(void);
void page_fault(void);
void coprocessor_error(void);
void reserved(void);

/*
 *
 *   中断可分为如下几类：
 *  1.programmed exception－用于实现系统调用，进程自愿进入核心态以请求服务
 *  2.fault－潜在可恢复错误，一般只显示错误信息，有时结束程序
 *  3.abort－致命的不可恢复错误，碰到此类错误，要么结束进程，要么死机或停机
 *  4.trap－执行调试指令时触发的，用于调试目的
 *
 */
 
 /*error msg for int 0 - int 19*/
private char *err_msg[] = {"#DE-divide error",
                           "#DB-reserved",
                           "NMI",
                           "#BP-breakpoint",
                           "#OF-overflow",
                           "#BR-bound range exceeded",
                           "UD-invalid opcode(undefined opcode)",
                           "#NM-device not available(not math coprocessor)",
                           "#DF-double fault",
                           "coprocessor segment overrun(reserved)",
                           "#TS-invalid tss",
                           "#NP-segment not present",
                           "#SS-stack-segment fault",
                           "#GP-general protection",
                           "#PF-page fault",
                           "intel reserved",
                           "#MF-x87 FPU floating-point error(math fault)",
                           "#AC-alignment check",
                           "#MC-machine check",
                           "#XF-SIMD floating-point exception"
};
 
 /*通用处理，这里先给出原型，以后再写，因为现在还没有打印函数*/
private void do_int(int vector,u32 esp,u32 err_code)
{
    printfs(err_msg[vector]);
}

public void do_divide_error(u32 esp,u32 err_code)
{
    do_int(NR_INT_DIVIDE_ERROR,esp,err_code);
}

public void do_single_step(u32 esp,u32 err_code)
{
    do_int(NR_INT_SINGLE_STEP,esp,err_code);
}

public void do_nmi(u32 esp,u32 err_code)
{
    do_int(NR_INT_NMI,esp,err_code);
}

/*int 3-5可在用户模型下调用，特权级是3*/
public void do_int3(u32 esp,u32 err_code)
{
    do_int(NR_INT_INT3,esp,err_code);
}

public void do_overflow(u32 esp,u32 err_code)
{
    do_int(NR_INT_OVERFLOW,esp,err_code);
}

public void do_bounds_check(u32 esp,u32 err_code)
{
    do_int(NR_INT_BOUNDS_CHECK,esp,err_code);
}

public void do_invalid_opcode(u32 esp,u32 err_code)
{
    do_int(NR_INT_INVALID_OPCODE,esp,err_code);
}

public void do_coprocessor_not_avl(u32 esp,u32 err_code)
{
    do_int(NR_INT_COPROCESSOR_NOT_AVL,esp,err_code);
}

public void do_double_fault(u32 esp,u32 err_code)
{
    do_int(NR_INT_DOUBLE_FAULT,esp,err_code);
}

public void do_coprocessor_seg_overrun(u32 esp,u32 err_code)
{
    do_int(NR_INT_COPROCESSOR_SEG_OVERRUN,esp,err_code);
}

public void do_invalid_tss(u32 esp,u32 err_code)
{
    do_int(NR_INT_INVALID_TSS,esp,err_code);
}

public void do_seg_not_present(u32 esp,u32 err_code)
{
    do_int(NR_INT_SEG_NOT_PRESENT,esp,err_code);
}

public void do_stack_exception(u32 esp,u32 err_code)
{
    do_int(NR_INT_STACK_EXCEPTION,esp,err_code);
}

public void do_general_protection(u32 esp,u32 err_code)
{
    do_int(NR_INT_GENERAL_PROTECTION,esp,err_code);
}

public void do_coprocessor_error(u32 esp,u32 err_code)
{
    do_int(NR_INT_COPROCESSOR_ERROR,esp,err_code);
}

public void do_reserved(u32 esp,u32 err_code)
{
    do_int(15,esp,err_code);
}

/*中断初始化，现在并没有写完整，以后再补充*/
/*
*
*   intel手册中定义的interrupt设置成中断门，exception设置为陷井门，特权级都是0
*   int3-5设置成特权级为3的陷井门，系统调用全部设置成陷井门，特权极为3
*
*/
public void int_init()
{
    int i;
    
    set_trap_gate(NR_INT_DIVIDE_ERROR,divide_error);
    set_trap_gate(NR_INT_SINGLE_STEP,single_step);
    set_trap_gate(NR_INT_NMI,nmi);
    set_system_gate(NR_INT_INT3,int3);
    set_system_gate(NR_INT_OVERFLOW,overflow);
    set_system_gate(NR_INT_BOUNDS_CHECK,bounds_check);
    set_trap_gate(NR_INT_INVALID_OPCODE,invalid_opcode);
    set_trap_gate(NR_INT_COPROCESSOR_NOT_AVL,coprocessor_not_avl);
    set_trap_gate(NR_INT_DOUBLE_FAULT,double_fault);
    set_trap_gate(NR_INT_COPROCESSOR_SEG_OVERRUN,coprocessor_seg_overrun);
    set_trap_gate(NR_INT_INVALID_TSS,invalid_tss);
    set_trap_gate(NR_INT_SEG_NOT_PRESENT,seg_not_present);
    set_trap_gate(NR_INT_STACK_EXCEPTION,stack_exception);
    set_trap_gate(NR_INT_GENERAL_PROTECTION,general_protection);
    set_trap_gate(NR_INT_PAGE_FAULT,page_fault);
    set_trap_gate(15,reserved);
    set_trap_gate(NR_INT_COPROCESSOR_ERROR,coprocessor_error);
   
    for(i = 17 ; i <= 32 ; i ++)
    {
        set_trap_gate(i,reserved);
   }
   
   /*其它未尽的中断，以后写相关代码时再添加*/
}
