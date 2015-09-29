/***************************************************************************
 *
 *  aifs.h-抽象文件系统接口，用于各种设备与文件系统之间
 *  的抽象，比如对于块设备，在block_dev.c中定义了块设备操
 *  作函数block_write和block_read，这些函数用于文件系统
 *  与其它模块之间的接口函数read和write中，提供整体文件系
 *  统的二级抽象。
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _AIFS_H
#define _AIFS_H

/*
*   块设备
*   在block_dev.c中定义
*   对函数详细的说明参见对应实现文件中的注释
*/
public int block_write(int dev,int *pos,char *buffer,int count);
public int block_read(int dev,int *pos,char *buffer,int count);

/*
*   文件
*   在file_dev.c中定义
*/
public int file_write(struct_mem_inode *inode,struct_file *file,char *buffer,int count);
public int file_read(struct_mem_inode *inode,struct_file *file,char *buffer,int count);

/*
*字符设备，显示器和键盘
*在chr_dev.c中实现
*/
public int chr_write(int dev,char *buffer,int count);
public int chr_read(int dev,char *buffer,int count);

#endif
