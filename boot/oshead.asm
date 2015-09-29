; -----------------------------------------------------------------
;   oshead.asm 内核代码的起始位置
; -----------------------------------------------------------------

%include "i386def.inc"

extern kmain
extern sys_stack

global sys_pgdir
global sys_pgtbl
global sys_gdt,sys_gdt_48
global sys_idt,sys_idt_48

[SECTION .StartUp32]
[BITS 32]
        ;设置保护模式下的段选择符为0x10
        mov     eax,0x10
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax
        mov     ss,ax
        mov     esp,sys_stack + 4096
        
        ;加载gdt和中断
        lgdt [sys_gdt_48]
        
        ;加载中断
        lidt [sys_idt_48]
        
        ;设置CR0的位16WP为1，以允许写保护
        xor     eax,eax
        mov     eax,cr0
        or      eax,0x10000
        mov     cr0,eax
        
        ;下面开始设置页目录和页表
        ;首先清零
        mov     ecx,17 * 1024 * 4
        xor     eax,eax
        mov     edi,sys_gdt
        cld
        rep stosb
        ;初始化页目录
        mov     ecx,16
        xor     eax,eax
        xor     ebx,ebx
setdir_again:
        mov     ebx,eax
        shl     ebx,12
        add     ebx,sys_pgtbl + 7
        mov     dword [sys_pgdir + eax * 4],ebx
        inc     eax
        loop    setdir_again
        ;下面开始填写所有页表的内容
        mov     ecx,16 * 1024
        xor     eax,eax
        xor     ebx,ebx
settbl_again:
        mov     ebx,eax
        shl     ebx,12
        add     ebx,7
        mov     [sys_pgtbl + eax],ebx
        inc     eax
        loop    settbl_again
        ;下面开启分页，在cr0
        mov     eax,sys_pgdir
        mov     cr3,eax
        mov     eax,cr0
        or      eax,0x80000000
        mov     cr0,eax
        call    kmain
        
label_sys_end:
        jmp     label_sys_end
        
;空中断函数
null_int:
INT_HANDLER     equ null_int - $$
        iret
        
[SECTION .sysdata]
;以下是系统代码段和数据段的GDT，起始地址为0
sys_gdt:            
;                      base       limit         segattr      attribute
        descriptor      0,           0,              0,            0           ;第一个描述符必须为全零
kernel_code:
        descriptor      0, SYS_CODELEN,  DA_G4KB+DA_32,    10011010b           ;32位，DPL=0,在内存中，代码段，非一致，可读，未访问过
kernel_data:
        descriptor      0, SYS_DATALEN,  DA_G4KB+DA_32,    10010010b           ;段上部界限为4GB,在内存，DPL=0，数据段，可读可写，未访问过
        descriptor      0,          0,              0,              0          ;for ldt
        descriptor      0,          0,              0,              0          ;for tss

GDT_LEN     equ $-sys_gdt
KERNEL_CS   equ kernel_code - sys_gdt
KERNEL_DS   equ kernel_data - sys_gdt

sys_gdt_48:
        dw  GDT_LEN - 1
        dd  sys_gdt

;中断向量表
sys_idt:
%rep 255
        gate INT_HANDLER,0,0,DA_GATE_INT
%endrep

IDT_LEN equ $-sys_idt

sys_idt_48:
        dw  IDT_LEN - 1
        dd  sys_idt
        
;org 0x1000
sys_pgdir:
        times 1024 dd 0
sys_pgtbl:
        times 1024*16 dd 0
