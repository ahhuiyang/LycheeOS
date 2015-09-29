;******************************************************************************************
;
;   i8259.asm-IRQ0-IRQ15中断处理程序接口
;       
;   Copyright (C) 2010 杨习辉
;
;******************************************************************************************/

extern irq_handler_table

global irq_0,irq_1,irq_2,irq_3,irq_4,irq_5,irq_6,irq_7
global irq_8,irq_9,irq_10,irq_11,irq_12,irq_13,irq_14,irq_15

;所有的irq中断都 没有错误号，包括一些使用int n指令发出调用的中断
;即使按照定义有错误号存在

irq_0:
    push    dword [irq_handler_table + 0 * 4]   ;将中断处理程序地址入栈
    push    0x20                                ;8259控制端口地址
    jmp     handle_irq
irq_1:
    push    dword [irq_handler_table + 1 * 4]
    push    0x20
    jmp     handle_irq
irq_2:
    push    dword [irq_handler_table + 2 * 4]
    push    0x20
    jmp     handle_irq
irq_3:
    push    dword [irq_handler_table + 3 * 4]
    push 0x20
    jmp     handle_irq
irq_4:
    push    dword [irq_handler_table + 4 * 4]
    push    0x20
    jmp     handle_irq
irq_5:
    push    dword [irq_handler_table + 5 * 4]
    push    0x20
    jmp     handle_irq
irq_6:
    push    dword [irq_handler_table + 6 * 4]
    push    0x20
    jmp     handle_irq
irq_7:
    push    dword [irq_handler_table + 7 * 4]
    push    0x20
    jmp     handle_irq
irq_8:
    push    dword [irq_handler_table + 8 * 4]
    push    0xA0
    jmp     handle_irq
irq_9:
    push    dword [irq_handler_table + 9 * 4]
    push    0xA0
    jmp     handle_irq
irq_10:
    push    dword [irq_handler_table + 10 * 4]
    push    0xA0
    jmp     handle_irq
irq_11:
    push    dword [irq_handler_table + 11 * 4]
    push    0xA0
    jmp     handle_irq
irq_12:
    push    dword [irq_handler_table + 12 * 4]
    push    0xA0
    jmp     handle_irq
irq_13:
    push    dword [irq_handler_table + 13 * 4]
    push    0xA0
    jmp     handle_irq
irq_14:
    push    dword [irq_handler_table + 14 * 4]
    push    0xA0
    jmp     handle_irq
irq_15:
    push    dword [irq_handler_table + 15 * 4]
    push    0xA0
    jmp     handle_irq
    
;下面是真正的处理过程
handle_irq:
    ;首先保存现场
    xchg    eax,[esp + 4]           ;现在中断处理程序的地址在eax了
    xchg    ebx,[esp]
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp
    push    ds
    push    es
    push    gs
    push    fs
    
    ;将这些段寄存器指向内核，gs留为用户进程与内核通信用
    push    eax
    mov     eax,0x10
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     eax,0x17
    mov     gs,ax
    pop     eax
    
    ;将中断之前的系统运行特权级压堆栈
    mov     edx,[esp + 12*4]
    and     edx,3
    push    edx
    ;发出EOI
    mov     al,0x20
    out     0x20,al
    ;调用中断处理程序
    call    eax
    ;返回
    add     esp,4                   ;跳过压入的特权级
    pop     fs
    pop     gs
    pop     es
    pop     ds
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ;中断返回
    iret
    
    
