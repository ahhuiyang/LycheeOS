/***************************************************************************
 *
 *  ftype.h-文件类型常量定义及宏
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _FTYPE_H
#define _FTYPE_H

/*文件类型掩码*/
#define FILE_TYPE           0xF000
#define FILE_TYPE_CHR       0x1000
#define FILE_TYPE_BLK       0x2000
#define FILE_TYPE_DIR       0x4000
#define FILE_TYPE_NORMAL    0x8000

/*判断文件类型的宏*/
#define IS_CHR(mode) (((mode) & FILE_TYPE) == FILE_TYPE_CHR)
#define IS_BLK(mode) (((mode) & FILE_TYPE) == FILE_TYPE_BLK)
#define IS_DIR(mode) (((mode) & FILE_TYPE) == FILE_TYPE_DIR)
#define IS_NORMAL(mode) (((mode) & FILE_TYPE) == FILE_TYPE_NORMAL)

/*文件控制掩码*/
#define FILE_CTL_ACCESS     0x0FF       /*文件读写控制掩码*/
#define FILE_CTL_OPR        0x0FF00     /*文件操作相关控制位掩码*/

/*读写控制*/
#define FILE_CTL_READONLY   0x00        /*读写控制－－只读*/
#define FILE_CTL_WRITEONLY  0x01        /*读写控制－－只写*/
#define FILE_CTL_RW         0x02        /*读写控制－－读写*/
#define FILE_CTL_EXEC       0x04        /*读写控制－－可执行（可读、不可写）*/

/*
*操作控制
*如果什么标志都没有，默认是：
*       当文件不存在时，读、写操作返回失败
*       当文件存在时，写操作从文件指针处开始覆盖写入
*/
#define FILE_CTL_CREATE     0x00100     /*如果文件不存在，就创建新文件*/
#define FILE_CTL_EXCL       0x00200     /*独占使用文件*/
#define FILE_CTL_TRUNC      0x00400     /*若文件已存在且是写操作，则长度截断为零*/
#define FILE_CTL_APPEND     0x00800     /*如果是写操作，则向文件尾追加内容*/

#endif
