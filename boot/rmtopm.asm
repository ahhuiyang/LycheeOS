;***************************************************************************
; 
;   kernel.h-内核全部变量定义
;   Copyright (C) 2010 杨习辉
 ;**************************************************************************/
[SECTION .code]
[BITS 16]
code_begin:
        mov ax,0x9000
        mov ds,ax
        
        ;获取扩展内存大小
        mov ah,0x88
        int 0x15
        mov [0],ax
        
        ;取显示状态
        mov ah,0x12
        mov bl,0x10
        int 0x10
        mov [2],ax
        mov [4],bx
        
        ;保存屏幕列为80，这个应该不会错
        mov word [6],0x50
        
        ;取得显示卡当前显示 模式，取得的参数包括字符列数，显示模式
        mov ah,0x0F
        int 0x10
        mov [8],ah
        mov [10],al
        
        ;取得当前光标位置
        mov ah,0x03
        xor bh,bh
        int 0x10
        mov [12],dl
        mov [14],dh
        
;取在BIOS中断处x41的第一个硬盘信息，共16个字节
        mov ax,0x0000
        mov ds,ax
        mov ax,0x9000
        mov es,ax
        mov si,[4 * 0x41]
        mov di,0x20
        mov cx,0x10
        cld
        rep movsb
        
;取第二个硬盘信息，位于0x46
        mov ax,0x0000
        mov ds,ax
        mov ax,0x9000
        mov es,ax
        mov si,[4 * 0x46]
        mov di,0x30
        mov cx,0x10
        cld
        rep movsb
        
;现在利用BIOS中断判断是否有第二个硬盘，若没有则将第二个硬盘数据清零
        mov ax,0x01500
        mov dl,0x81
        int 0x13
        jnc move_to_protect
        
;下面清理第二个硬盘参数
        mov ax,0x9000
        mov es,ax
        mov di,0x30
        mov cx,0x10
        mov al,0x00
        cld
        rep stosb
        
;下面正式进入保护模式
move_to_protect:
        ;不允许中断
        cli
        ;开启A20地址线
        in  al,0x92
        or  al,0x02
        out 0x92,al
        ;准备切换到保护模式，设置PE位
        mov eax,cr0
        or  eax,1
        mov cr0,eax
        ;必须再来一个跳转，才能真正进入保护模式
        jmp dword 0x08:0
