;***************************************************************************
;
;   pf.asm-page fault中断处理函数(int 14)
;   Copyright (C) 2010 杨习辉
;***************************************************************************

;中断时的堆栈情况请参考kernel/interrupt.asm中的注释

;memory.c中的do_wp_page和do_no_page函数
extern do_wp_page
extern do_no_page

;导出中断处理函数
global page_fault

page_fault:
    xchg    eax,[esp]       ;将出错码交换到eax，实际就是出错的3个标志位
    push    ecx
    push    edx
    push    ds
    push    es
    push    fs
    push    gs
    mov     edx,cr2
    push    edx             ;address,线性地址
    push    eax             ;err_code,出错码
    
    test    eax,1           ;测试p位，如果为零，则是缺页异常，否则跳转
    jnz     wp_page         ;上述and操作结果不为零，也就是说P位为1，则跳
    call    do_no_page
    jmp     label_exit
    
wp_page:
    call    do_wp_page
    
label_exit:
    add     esp,8           ;压入堆栈的两个参数
    pop     gs
    pop     fs
    pop     es
    pop     ds
    pop     edx
    pop     ecx
    pop     eax
    iret
