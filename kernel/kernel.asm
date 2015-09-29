;***************************************************************************
; 
;   kernel.asm-内核操作，但是无法用C语言描述的功能
;   Copyright (C) 2010 杨习辉
; **************************************************************************/

extern sys_pgdir

global reload_cr3
global move_to_user
global load_seg

;----------------------------------------------------------------
;public void reload_cr3(void)
;重新加载CR3，这样可以使CPU的TLB无效，然后CPU就要重新生成TLB中的内容了
;----------------------------------------------------------------
reload_cr3:
    push    eax
    mov     eax,sys_pgdir
    mov     cr3,eax
    pop     eax
    ret
;----------------------------------------------------------------
;public void load_seg(void)
;重新加载段寄存器，只给任务0使用
;----------------------------------------------------------------
load_seg:
    push    eax
    mov     eax,0x17
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     gs,ax
    pop     eax
    ret
 ;----------------------------------------------------------------
;public void load_ldt(u16 ldt)
;重新加载段寄存器，只给任务0使用
;----------------------------------------------------------------
load_ldt:
    push    ebp
    mov     ebp,esp
    
    push    eax
    xor     eax,eax
    mov     ax,[ebp + 8]
    lldt    ax
    
    pop     eax
    mov     esp,ebp
    pop     ebp
    
    ret
 ;----------------------------------------------------------------
;public void load_tr(u16 tss)
;重新加载段寄存器，只给任务0使用
;----------------------------------------------------------------
load_tr:
    push    ebp
    mov     ebp,esp
    
    push    eax
    xor     eax,eax
    mov     ax,[ebp + 8]
    ltr     ax
    
    pop     eax
    mov     esp,ebp
    pop     ebp
    
    ret;   
