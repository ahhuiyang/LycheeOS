;***************************************************************************
; 
;   syscall.asm-系统调用最外层接口，当发生系统调用
;   第一个调用的就是这个文件里的函数
;   Copyright (C) 2010 杨习辉
;**************************************************************************/

;定义在syscall.h中，系统调用函数表
extern sys_call_table
;定义在fork.c中
extern get_free_process
extern copy_process

global sys_call
global sys_fork

;系统调用总数，必须跟syscall.h中的NR_SYS_CALLS一致
NR_SYS_CALLS    equ     0

sys_call:
    ;合法性验证，系统调用号放在eax中
    cmp     eax,NR_SYS_CALLS - 1
    ja      .sys_call_ret
    cmp     eax,0
    jb      .sys_call_ret
    
    push    esi
    push    edi
    push    ebp
    push    ds
    push    es
    push    fs
    push    gs
    
    ;将这些段寄存器指向内核，gs留为用户进程与内核通信用
    push    eax
    mov     eax,0x10    ;内核段
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     eax,0x17    ;用户段
    mov     gs,ax
    pop     eax
    
    ;给真正的系统调用准备参数
    ;参数放在ebx,ecx,edx中，最多只能有三个参数
    ;如果参数过多，请设置指向一个数据结构的指针
    ;ebx,ecx,edx分别放着第一个，第二个，第三个参数
    
    push    edx
    push    ecx
    push    ebx
    
    ;真正的系统调用，调用过后，eax中有返回值
    call    [sys_call_table + eax*4]
    
    ;清理堆栈
    pop     ebx
    pop     ecx
    pop     edx
    pop     gs
    pop     fs
    pop     es
    pop     ds
    pop     ebp
    pop     edi
    pop     esi
    
.sys_call_ret:
    iret
;----------------------------------------------------------------
;public int sys_fork(void)
;创建一个新进程 
;----------------------------------------------------------------
sys_fork:
    call    get_free_process    ;查找空闲进程的索引
    cmp     eax,0               ;如果返回值为零，则出错
    je      .label_exit         ;返回值为零退出
    call    copy_process        ;复制进程

.label_exit:
    ret
