/***************************************************************************
 *
 *  fconst.h-文件系统常量定义
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _FCONST_H
#define _FCONST_H

/*const*/
#define INODES_PER_BLOCK    (BLOCK_SIZE / sizeof(struct_inode))
#define DIRS_PER_BLOCK      (BLOCK_SIZE / sizeof(struct_dir_entry))

/*i节点位图和数据块位图所用到的内存缓冲块数*/
#define INODE_MAP_SLOTS     8
#define DATA_MAP_SLOTS      8

/*一些固定块在磁盘上的位置，以块为单位*/
#define FS_POS_BOOT         0
#define FS_POS_SUPER        1

#endif
