;段描述符
;usage:
;      descriptor base,limit,segattr,attribute
;      base:dd
;      limit:dd(only low 20 bits are avaiable)
;      segattr:db
;      attribute:db(lower 4 bits of higher byte are 0)
%macro descriptor 4
			dw		%2 & 0x0ffff
			dw		%1 & 0x0ffff
			db		(%1 >> 16) & 0x0ff
			db		%4 & 0x0ff
			db		(%3 & 0xf0) | ((%2 >> 16) & 0x0f)
			db		(%1 >> 24) & 0x0f
%endmacro

;门描述符
;usage:
;      gate offset,selector,pcount,attribute
;      offset:dd
;      selector:dw
;			   pcount:db
;      attribute:dw
%macro gate 4
			dw		%1 & 0x0ffff
			dw		%2 & 0x0ffff
			db		%3 & 0x0ff
			db		%4 & 0x0ff
			dw		(%1 >> 16) & 0x0ffff
%endmacro

;只用于数据、代码段，表明是32位数据或代码
DA_G4KB  equ   0x80     ;如果设置此位，则使用4KB的分配粒度，否则为字节
DA_32			equ		0x40     ;设置些位，使用32位段

;描述符属性，只用于代码、数据段，设置的是D/B位
;下图是属性字段
;跟代码段或数据段描述符的属性字段是一样的
;区别只是在8字节中的位置不同
;
;|---|------|---|------------|;| P | DPL  | S |    TYPE    |
;|___|______|___|____________|

;特权级
DA_DPL0		equ		0x00
DA_DPL1		equ		0x20
DA_DPL2		equ		0x40
DA_DPL3		equ		0x60

;系统段
DA_SYSSEG_LDT		equ		0x82
DA_SYSGATE_TSS	equ		0x89
;门
DA_GATE_TASK   equ   0x85
DA_GATE_CALL		equ		0x8C
DA_GATE_INT			equ		0x8E
DA_GATE_TRAP		equ		0x8F

;系统代码段长度
SYS_CODELEN     equ     4096*4   ;4KB,total = 16MB
;系统数据段长度
SYS_DATALEN     equ     4096*4   ;4KB,total = 16MB
