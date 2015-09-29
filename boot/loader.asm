; -----------------------------------------------------------------
;loader.asm   (C) 2010 yangxihui
;内核启动程序加载器，用于将rmtopm.bin和操作系统初始化头oshead加载到内存中
;
;机器刚启动时，处于实模式(Real Mode)，只能寻址1MB的内存，内存大致分布如下：
; 0x00000-0x003ff BIOS中断，共256个中断
; 0x00400-0x9ffff 用户可用内存空间
; 0xA0000-0xbffff 这一范围一般是用于显存空间的
; 0xC0000-0xfffff BIOS ROM
;
;因此，我只用从0x10000-0x9FFFF这段空间，而且linux系统也是这么用的
;由于开始要用到BIOS中断来读取磁盘，所以内存最低端不能使用
;
;BIOS将此段代码加载到物理地址为0x07c00的地方，为了给操作系统初始化程序预留
;一定的空间（512KB），这段程序将自身搬到0x90000的地方，并将rmtopm.bin从磁盘
;加载到0x90200的地方
;
;编译方法：
;    nasm loader.asm -o loader.bin
;
;注意：
;    必须将该段程序写到磁盘的第一个512B扇区
; -----------------------------------------------------------------

org 0x07c00

;预留，oshead的大小，以Byte计
OSSIZE          equ     512 * 1024
;rmtopm.bin的长度，以扇区为单位
RMTOPMLEN       equ     4
;系统的长度
OSLEN           equ     1024
;加载段地址
INITSEG         equ     0x07c0
;转移到的地址
LOADSEG         equ     0x9000
;rmtopm.bin的加载地址
RMTOPMSEG       equ     0x9020
;oshead初始加载地址
OSSEG           equ     0x1000

[SECTION .CODE]
[BITS 16]
            ;首先将自身移到到0x90000位置处
            mov     ax,INITSEG
            mov     ds,ax
            mov     ax,LOADSEG
            mov     ds,ax
            mov     cx,256          ;每次移到两字节，需要移动256次
            xor     si,si
            xor     di,di
            rep movsw               ;重复执行，直到cx=0，复制完毕
            jmp     LOADSEG:MoveAfter
            ;从下面开始，代码从0x90000段开始执行
MoveAfter:
            ;由于移动到了0x90000处，因此要重置段寄存器
            mov     ax,cs
            mov     ds,ax
            mov     es,ax
            ;置堆栈到0x9ffff处
            mov     ss,ax
            mov     sp,0xffff
            ;要加载rmtopm.bin到内存了，现在假设只占用四个扇区，等以后实际写出来，再稍加修改
            ;这个过程要利用BIOS的int 13h中断
.Load_Rmtopm:
            mov     dx,0x0000       ;DH-Driver Number(0-3)，默认为0，即当前引导磁盘加载，DL-磁头号，为0
            mov     cx,0x0002       ;CH-磁道柱面号的低8位,CL-扇区号（0－5位）和磁道柱面号的高2位(6-7位)
            mov     bx,0x0200       ;es:bx-数据读入保存到内存的位置为0x9000:0x2000
            mov     ax,0x0200 + RMTOPMLEN   ;AH-02H是读磁盘调用功能号，AL-要读的扇区数
            int     13h             ;开始读，如果出错，CF置位
            jnc     .load_rmtopm_ok  ;有可能马达还没有旋转起来，因此至少要重复三次，在这里，如果不成功，则反复读，没有退路
            ;如果不成功，则复位磁盘后重新读取
            xor     dx,dx
            xor     cx,cx
            xor     bx,bx
            xor     ax,ax
            jmp     .Load_Rmtopm

;下面要加载系统内核
.load_rmtopm_ok:
            mov     ax,OSSEG
            mov     es,ax
            xor     bx,bx
            call    read_os

;取得硬盘参数
            xor     ax,ax
            mov     es,ax
            mov     di,word 0x106
            mov     al,byte [es:di]
            mov     byte [heads],al
            mov     di,word 0x112
            mov     al,byte [es:di]
            mov     byte [sectors_per_track],al  
;下面的代码用户将系统内核读入内存
read_os:
            ;使用绝对扇区号，此扇区号从零开始
            mov     dword [sector],3
            mov     ax,OSSEG
            mov     es,ax
            xor     bx,bx
.read_loop:
            ;首先求扇区号
            
            mov     ax,word [sector]
            shr     dword [sector],16
            mov     dx,word [sector]
            div     word [sectors_per_track]
            ;现在，DX=扇区号-1
            inc     dx
            ;同时，AX=磁道数，现在求柱面号和磁头号
            div     byte [heads]
            ;现在，AL=柱面号，AH=磁头号
            mov     ch,al
            mov     cl,dl
            mov     dh,ah
            mov     ah,0x02
            mov     al,1
            mov     dl,0x80
            int     0x13
            
            inc     byte [reads]
            inc     dword [sector]
            add     bx,512
            
            mov     ax,es
            cmp     byte [reads],128
            jb      .read_loop
            cmp     ax,0x8000
            jae     .label_end_loader
            mov     byte [reads],0
            mov     bx,0
            add     ax,0x1000
            mov     es,ax
            jmp     .read_loop
            
.label_end_loader:
            jmp     RMTOPMSEG:0x0000
sector:
    dd  0
sectors_per_track:
    dw  0
heads:
    db  0
reads:
    db  0
