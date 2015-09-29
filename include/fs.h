/***************************************************************************
 *
 *  fs.h-文件系统常量和结构定义
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#ifndef _FS_H
#define _FS_H

#include "kernel.h"
#include "type.h"
#include "fconst.h"
#include "ftype.h"
#include "buffer.h"

/*设备在内核中的编号，跟minix的相同
*   0-unused
*   1-/dev/mem
*   2-/dev/usb
*   3-/dev/hd
*   4-/dev/tty serial
*   5-/dev/tty
*   6-/dev/lp
*   7-unnamed pipes
*/

/*宏定义*/
/*----------------------------------------------------------------------*/
/*读写标识*/
#define READ    0
#define WRITE   1

/*文件名*/
#define NAME_LEN    14
#define ROOT_INODE  1

/*inode节点数和超级块数*/
#define NR_INODE    32
#define NR_SUPER    8
#define NR_FILE     64
#define NR_OPEN     8

/*直接块号、一次间接块号、二次间接块号...*/
#define INDEX_LEVEL1    7
#define INDEX_LEVEL2    8

/*magic number*/
#define SUPER_MAGIC     0x137F

/*磁盘上的i节点结构*/
typedef struct _struct_inode
{
    /*以下内容在磁盘上和内存中是一样的*/
    unsigned int mode;          /*文件类型和属性*/
    unsigned int size;          /*文件长度，字节*/
    unsigned int modify_time;   /*修改时间*/
    unsigned int create_time;   /*创建时间*/
    unsigned int last_access;   /*最近访问时间*/
    unsigned int nlinks;        /*有多少目录项指向此inode*/
    unsigned short data[9];     /*盘块号数组，0-6直接，7一级，8二级*/
}struct_inode;

typedef struct _struct_mem_inode
{
    /*以下内容在磁盘上和内存中是一样的*/
    unsigned int mode;          /*文件类型和属性*/
    unsigned int size;          /*文件长度，字节*/
    unsigned int modify_time;   /*修改时间*/
    unsigned int create_time;   /*创建时间*/
    unsigned int last_access;   /*最近访问时间*/
    unsigned int nlinks;        /*有多少目录项指向此inode*/
    unsigned short data[9];     /*盘块号数组，0-6直接，7一级，8二级*/
    /*以下内容只出现在内存中*/
    struct_process *proc_wait;  /*等待此i节点的进程*/
    unsigned int dev;           /*i节点所在的设备号*/
    unsigned short inode_num;    /*i节点号*/
    unsigned short count;       /*i节点被引用的次数，0表示空闲*/
    unsigned char locked;       /*i节点被锁定标志*/
    unsigned char dirty;        /*i节点被修改标志*/
    unsigned char uptodate;       /*i节点有效标志*/
    unsigned char mount;        /*i节点安装了其它文件系统标志*/
}struct_mem_inode;

/*
*
*磁盘超级块结构，位于磁盘块1（从0开始）最大只能支持64MB大小的文件
*这个限制主要是因为目前的内存管理还不完善
*/
typedef struct _struct_super_block
{
    u16 nr_inodes;         /*i节点数*/
    u16 nr_blocks;    /*数据块数（数据块）*/
    u16 imap_blocks;       /*i节点位图所占块数*/
    u16 dmap_blocks;       /*数据块位图所占块数*/
    u16 first_data_block;  /*第一个数据块的块号，在整个磁盘内从0开始*/
    u32 max_size;          /*文件的最大长度*/
    u16 magic;             /*用于数据校验*/
}struct_super_block;

/*在内存中的超级块结构*/
typedef struct _struct_mem_super_block
{
    /*以下内容磁盘和内存中相同*/
    u16 nr_inodes;         /*i节点数*/
    u16 nr_blocks;    /*数据块数（数据块）*/
    u16 imap_blocks;       /*i节点位图所占块数*/
    u16 dmap_blocks;       /*数据块位图所占块数*/
    u16 first_data_block;  /*第一个数据块的块号，在整个磁盘内从0开始*/
    u32 max_size;          /*文件的最大长度*/
    u16 magic;             /*用于数据校验*/
    /*以下内容仅在内存中存在*/
    struct_mem_buffer *imap[8];  /*i节点位图在高速缓冲块中的指针*/
    struct_mem_buffer *dmap[8];  /*数据块位图在高速缓冲块中的指针*/
    unsigned int dev;                    /*超级块所在的设备号*/
    struct_mem_inode *root_inode;        /*被安装文件系统根节点inode*/
    struct_mem_inode *imount;            /*该文件系统被安装到的i节点*/
    struct_process *proc_wait;           /*等待本超级块的进程指针*/
    unsigned char locked;                /*锁定标志*/
    unsigned char read_only;             /*只读标志*/
    unsigned char dirty;                 /*已被修改标志*/
    unsigned char uptodate;              /*有效标志*/    
    
}struct_mem_super_block;

/*目录项*/
typedef struct _struct_dir_entry
{
    unsigned short inode;
    char name[NAME_LEN];
}struct_dir_entry;

/*文件*/
typedef struct _struct_file
{
    unsigned short mode;        /*文件类型和访问属性，比如只读、只写，对应inode.mode*/
    unsigned short flags;       /*文件打开和控制标志，打开时设置*/
    unsigned short count;       /*此文件被引用的次数*/
    unsigned int offset;        /*当前读写偏移位置*/
    struct_mem_inode *inode;    /*对应内存i节点*/
}struct_file;

public extern struct_mem_super_block super_block[NR_SUPER];

/*函数原型，定义在buffer.c中*/
public void lock_mem_buffer(struct_mem_buffer *mbuffer);
public void unlock_mem_buffer(struct_mem_buffer *mbuffer);
public  void wait_on_buffer(struct_mem_buffer *mbuffer);

/*----------------------------------------------------------------------*/

/*函数原型*/
/*----------------------------------------------------------------------*/

/*以下函数定义在super.c中*/
public struct_mem_super_block *get_super(int dev);

/*以下函数定义在bitmap.c中*/
public void free_block(int dev,int block);
public int new_block(int dev);
public void free_inode(struct_mem_inode *inode);
public void new_indoe(int dev);

/*以下函数定义在truncate.c中*/
public void truncate(struct_mem_inode *inode);

/*以下函数定义在inode.c中*/
public struct_mem_inode *get_inode(int dev,int nr);
public struct_mem_inode *get_empty_inode(void);
public void put_inode(struct_mem_inode *inode);
public void sync_inodes(void);
public void invalid_inodes(int dev);

/*以下函数定义在namei.c中*/
public struct_mem_inode *namei(char *pathname);
/*----------------------------------------------------------------------*/

#endif
