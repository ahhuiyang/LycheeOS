/***************************************************************************
 *
 *  const.h-常量定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 #ifndef _CONST_H
 #define _CONST_H
 
 /*---显示器控制---------------------------------*/
 
/*单色显示器*/
#define MONO_MEM_START 0xb0000
#define MONO_EGA_MEM_SIZE (32 * 1024)   /*EGA Mono,32KB*/
#define MONO_MDA_MEM_SIZE (8 * 1024)    /*MDA,8KB*/
#define MONO_ADDR_REG 0x3b4
#define MONO_DATA_REG 0x3b5

/*增强型显示器*/
#define CRT_MEM_START 0xb8000
#define CRT_EGA_MEM_SIZE (32 * 1024)    /*32KB*/
#define CRT_VGA_MEM_SIZE (32 * 1024)    /*32KB*/
#define CRT_CGA_MEM_SIZE (8 * 1024)     /*8KB*/
#define CRT_ADDR_REG 0x3d4
#define CRT_DATA_REG 0x3d5

/*CRT Controller Data Registers*/
/*显示内存起始位置（高位，写）*/
#define VIDEO_START_ADDR_H          0xC
/*显示内存起始位置（低位，写）*/
#define VIDEO_START_ADDR_L          0xD
/*光标当前位置（高16位，读写）*/
#define VIDEO_CURSOR_H              0xE
/*光标当前位置（低16位，读写）*/
#define VIDEO_CURSOR_L              0xF

/*显示器类型*/
#define VIDEO_MDA   1
#define VIDEO_CGA   2
#define VIDEO_EGAM  3
#define VIDEO_VGA   4

/*---8259A-------------------------------------*/
#define I8259_MASTER_CTL    0x20
#define I8259_MASTER_MASK   0x21
#define I8259_SLAVE_CTL     0xA0
#define I8259_SLAVE_MASK    0xA1

/*IRQ0-IRQ15对应int 20-int 2F*/
#define INT_IRQ0            0x20
#define INT_IRQ8            0x28

#endif
 
