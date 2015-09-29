;***************************************************************************
;
;   interrupt.asm-int0-int16中断处理接口
;   Copyright (C) 2010 杨习辉
;***************************************************************************

;无出错号，无特权级变化堆栈如下
; |-----------|                        
; eflags
; |-----------|
; cs
; |-----------|
; eip        <--esp
; |-----------|

;有出错号，无特权级变化，堆栈如下：


;没有出错号，有特权级变化，堆栈如下：
; |-----------|
; ss(外层)
; |-----------|
; esp(外层)
; |-----------|
; eflags
; |-----------|
; cs
; |-----------|
; eip        <--esp
; |-----------|

;有出错号，有特权级变化，堆栈如下：
; |-----------|
; ss(外层)
; |-----------|
; esp(外层)
; |-----------|
; eflags
; |-----------|
; cs
; |-----------|
; eip
; |-----------|
; error code <--esp
; |-----------|

;外部符号，这些中断真实的处理函数
;每个中断一个处理函数可以让中断变得比较强大
;这些函数在interrupt.c中实现                                          ;向量号      描述                      类型                      出错码 
extern do_divide_error                  ;0      除法错                 fault       无
extern do_single_step                   ;1      调试异常            falut\trap  无 
extern do_nmi                           ;2      非屏蔽中断        interrupt   无
extern do_int3                          ;3      int3       trap        无
extern do_overflow                      ;4      溢出                     trap        无
extern do_bounds_check                  ;5      越界                     fault       无
extern do_invalid_opcode                ;6      无效操作码        fault       无
extern do_coprocessor_not_avl           ;7      无数学协处理器fault       无
extern do_double_fault                  ;8      双重错误             abort      有(0)
extern do_coprocessor_seg_overrun       ;9      协处理器段越界fault       无
extern do_invalid_tss                   ;10     无效的TSS   fault       有
extern do_seg_not_present               ;11     段不存在             fault       有
extern do_stack_exception               ;12     堆栈错误             fault       有
extern do_general_protection            ;13     常规保护错误     fault       有
extern do_page_fault                    ;14     页错误                  fault       有
extern do_coprocessor_error             ;16     x87FPU错          fault       无
extern do_reserved                      ;暂时没有中断处理程序的函数执行此函数

;导出符号,中断的最外层
global divide_error,single_step,nmi,int3,overflow
global bounds_check,invalid_opcode,coprocessor_not_avl
global double_fault,coprocessor_seg_overrun,invalid_tss,seg_not_present
global stack_exception,general_protection,coprocessor_error
global reserved

divide_error:
    push    do_divide_error
    jmp     no_error_code

single_step:
    push    do_single_step
    jmp     no_error_code
nmi:
    push    do_nmi
    jmp     no_error_code
int3:
    push    do_int3
    jmp     no_error_code
overflow:
    push    do_overflow
    jmp     no_error_code
bounds_check:
    push    do_bounds_check
    jmp     no_error_code
invalid_opcode:
    push    do_invalid_opcode
    jmp     no_error_code
coprocessor_not_avl:
    push    do_coprocessor_not_avl
    jmp     no_error_code
double_fault:
    push    do_double_fault
    jmp     error_code
coprocessor_seg_overrun:
    push    do_coprocessor_seg_overrun
    jmp     no_error_code
invalid_tss:
    push    do_invalid_tss
    jmp     error_code
seg_not_present:
    push    do_seg_not_present
    jmp     error_code
stack_exception:
    push    do_stack_exception
    jmp     error_code
general_protection:
    push    do_general_protection
    jmp     error_code
coprocessor_error:
    push    do_coprocessor_error
    jmp     no_error_code
reserved:
    push    do_reserved
    jmp     no_error_code
    
;下面的代码分为两种,有出错码和无出错码
;无出错码
no_error_code:
    xchg    eax,[esp]           ;将中断处理程序地址放入eax， 并将eax入栈
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp
    push    ds
    push    es
    push    ss
    push    fs
    push    gs
    
    ;将这些段寄存器指向内核，gs留为用户进程与内核通信用
    push    eax
    mov     eax,0x10
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     eax,0x17
    mov     gs,ax
    pop     eax
    
    push    0xffffffff          ;没有出错码，随便填充一个
    mov     edx,esp
    add     edx,4 * 13
    push    edx                 ;错误处理程序可以根据此值找出执行代码的CS:EIP,EFLAGS,SS:ESP
    call    eax
    add     esp,8               ;指向gs入栈处
    pop     gs
    pop     fs
    pop     ss
    pop     es
    pop     ds
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    iret
    
error_code:
    xchg    eax,[esp + 4]           ;将出错号放入eax， 并将eax入栈
    xchg    ebx,[esp]
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp
    push    ds
    push    es
    push    ss
    push    fs
    push    gs
    
    ;将这些段寄存器指向内核，gs留为用户进程与内核通信用
    push    eax
    mov     eax,0x10
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     eax,0x17
    mov     gs,ax
    pop     eax
    
    push    eax                 ;没有出错码，随便填充一个
    mov     edx,esp
    add     edx,4*13
    push    edx                 ;错误处理程序可以根据此值找出执行代码的CS:EIP,EFLAGS,SS:ESP
    call    ebx
    add     esp,8               ;指向gs入栈处
    pop     gs
    pop     fs
    pop     ss
    pop     es
    pop     ds
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    iret
