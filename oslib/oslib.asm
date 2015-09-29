;***************************************************************************
; 
;  oslib.asm-内核常用函数库，原型声明在include/oslib.h
;     
;  Copyright (C) 2010 杨习辉
 ;**************************************************************************
 
global enable_int
global disable_int
global nop
 
;设置eflags的if位，启用外部中断
enable_int:
    sti
    ret
    
;清除eflags的if位，禁用外部中断
disable_int:
    cli
    ret
;空操作
nop:
    nop
    ret
