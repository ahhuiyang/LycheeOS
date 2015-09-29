/***************************************************************************
 *
 *  namei.c-将完整路径的文件名对应到设备上的具体i节点号
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "type.h"
#include "blk.h"
#include "process.h"
#include "fs.h"
#include "string.h"

/*比较文件名或目录名是否与给定目录项中的文件名或目录名相同*/
private int is_nmatch(char *filename,struct_dir_entry *de,int len)
{
    int i,same;
    
    if(len > NAME_LEN || !de || (!de->inode))
    {
        return FALSE;
    }
    
    /*如果目录项中的名称字符串长度不等于长度len，则显然不相配，返回FALSE*/
    if(strlen(de->name) != len)
    {
        return FALSE;
    }
    
    /*首先假定相等，比较过程中若有一个字符不等，则结果为不匹配*/
    same = 1;
    /*比较长度*/
    for(i = 0 ; i < len ; i ++)
    {
        if(filename[i] != de->name[i])
        {
            same = 0;
            break;
        }
    }
    
    return (same)?TRUE:FALSE;
}

/*比较文件名和目录项名称是否相同，字符串必须是C字符串*/
private int is_match(char *filename,struct_dir_entry *de)
{
    int len,same,i;
    
    len = strlen(filename);
    if(len != strlen(de->name))
    {
        return FALSE;
    }
    
    same = 1;
    for(i = 0 ; i < len ; i ++)
    {
        if(filename[i] != de->name[i])
        {
            same = 0;
            break;
        }
    }
    
    return (same)?TRUE:FALSE;
}

/*在指定的目录中查找一个与给出的名字相匹配的一个目录项*/
private struct_dir_entry *find_entry(struct_mem_inode *dir,char *filename,struct_mem_buffer **ppmbuffer)
{
    int i,count,block;
    struct_mem_super_block *sb;
    struct_mem_buffer *mbuffer;
    struct_dir_entry *de;
    
    /*如果目录项为NULL或文件名长度为零，返回空值*/
    if(!dir || strlen(filename) == 0)
    {
        return NULL;
    }
    
    /*如果文件名长度大于规定的文件名长度，则返回NULL*/
    if(strlen(filename) > NAME_LEN - 1)
    {
        return NULL;
    }
    
    /*算出当前目录中有多少目录项(dir entry)*/
    count = dir->size / sizeof(struct_dir_entry);
    /*.代表当前目录，..代表父目录，但是有例外情况
    *一、目录是文件系统根目录，此时没有父目录
    *二、目录节点是根节点，如果此设备上的文件系统被安装在其它文件系统的节点上
    *   ..会将目录切换到安装点
    */
    if(strlen(filename) == 2 && filename[0] == '.' 
        && filename[1] == '.')
    {
        if(dir == current->root)
        {
            filename[1] = 0;
        }
        else if(dir->inode_num == ROOT_INODE)
        {
            /*获取文件系统超级块，以便获取安装点*/
            if(!(sb = get_super(dir->dev)))
            {
                printfs("can not retrive super block of dev %d\n",dir->dev);
                halt();
            }
            
            /*如果有安装点，则切换到安装点目录，否则 ..指向自身*/
            if(sb->imount)
            {
                /*首先是释放*/
                put_inode(dir);
                dir = sb->imount;
                dir->count ++;
            }
        }
    }
    
    /*现在可以开始读取目录文件*/
    /*如果第一个直接块号就为零，则返回空值*/
    if(!(block = dir->data[0]))
    {
        return NULL;
    }
    
    /*读取这一块内容*/
    if(!(mbuffer = bread(dir->dev,block)))
    {
        return NULL;
    }
    
    /*下面开始遍历目录文件了*/
    i = 0;
    de = (struct_dir_entry *)mbuffer->data;
    while(i < count)
    {
        /*如果当前块遍历完了，接着遍历下一块*/
        if((char *)de >= mbuffer->data + BLOCK_SIZE)
        {
            /*释放上一块的数据 */
            brelease(mbuffer);
            mbuffer = NULL;
            /*读取下一块*/
            if(!(block = ib_map(dir,i / DIRS_PER_BLOCK))
                || !(mbuffer = bread(dir->dev,block)))
            {
                /*当前项为空，接着遍历下一项，允许这样添加内容，数据不按顺序块存放*/
                i +=  DIRS_PER_BLOCK;
                continue;
            }
            
            /*读到了数据*/
            de = (struct_dir_entry *)mbuffer->data;
        }
        
        /*判断是否是要查找的目录，即名子是否匹配*/
        if(is_match(filename,de))
        {
            /*如果函数调用者传递的缓冲指针有效，则返回缓冲块*/
            if(ppmbuffer != NULL)
            {
                *ppmbuffer = mbuffer;
            }
            return de;
        }
        
        /*没找到，指向下一项*/
        de ++;
        i ++;
    }
    
    /*运行到这里，说明没有找到，因此返回空*/
    brelease(mbuffer);
    return NULL;  
}

/*往一个目录里添加一个目录项，根据指定的文件名创建这个目录项
*返回新添加的目录项，如果ppmbuffer不为NULL
*则将目录项所在的缓冲区通过此参数返回
*
*说明：不会为新目录项申请i节点
*/
private struct_dir_entry *add_entry(struct_mem_inode *dir,char *filename,struct_mem_buffer **ppmbuffer)
{
    int i,block;
    struct_mem_buffer *mbuffer;
    struct_dir_entry *de;
    
    if(!dir || filename[0] == 0 || strlen(filename) > NAME_LEN)
    {
        return NULL;
    }
    
    /*如果该具有相同名称的目录项已存在，则拒绝添加*/
    if(find_entry(dir,filename,NULL))
    {
        return NULL;
    }
    
    /*取目录的第一个数据块*/
    if(!(block = dir->data[0]))
    {
        return NULL;
    }
    
    /*取该数据块内容*/
    if(!(mbuffer = bread(dir->dev,block)))
    {
        return NULL;
    }
    
    i = 0;
    de = (struct_dir_entry *)mbuffer->data;
    /*循环查找空项*/
    while(TRUE)
    {
        /*如果当前块查找完了，则转到下一块*/
        if((char *)de >= mbuffer->data + BLOCK_SIZE)
        {
            brelease(mbuffer);
            mbuffer = NULL;
            /*如果没有对应的块，必须添加新块*/
            block = ib_map_new(dir,i / DIRS_PER_BLOCK);
            /*如果没申请到新块，则返回空*/
            if(!block)
            {
                return NULL;
            }
            /*读取盘块数据，如果读取失败，则读取下一磁盘块*/
            if(!(mbuffer = bread(dir->dev,block)))
            {
                i += DIRS_PER_BLOCK;
                continue;
            }
            de = (struct_dir_entry *)mbuffer->data;
        }
        
        /*如果当前经过的目录项大小等于目录文件大小
        *则说明没有由于删除目录而留下的空项位置
        *因此有效目录项位置紧接着的一个就是空项位置
        */
        if(i * sizeof(struct_dir_entry) >= dir->size)
        {
            de->inode = 0;  /*没有数据，不可用*/
            dir->size += sizeof(struct_dir_entry);
            dir->dirty = TRUE;
            dir->modify_time = CURRENT_TIME;
            dir->last_access = CURRENT_TIME;
        }
        
        /*
        *如果当前的目录项为空，则说明找到一个未使用的空目录项
        *注意，上面的情况也包括在内
        */
        if(!de->inode)
        {
            dir->modify_time = CURRENT_TIME;
            dir->last_access = CURRENT_TIME;
            
            /*复制文件名*/
            for(i = 0 ; i <NAME_LEN ; i ++)
            {
                de->name[i] = (filename[i])?filename[i]:0;
            }
            
            if(ppmbuffer != NULL)
            {
                *ppmbuffer = mbuffer;
            }
            
            return de;
        }
        
        /*继续查找*/
        de ++;
        i ++;
    }
    
    /*没找到，返回空值*/
    brelease(mbuffer);
    return NULL;
}

/*根据给定的路径，查找最 后的目录
*比如，对于 /cs/asm/a.asm
*将会返回asm目录的节点
*包含 一些特殊目录的处理，它们是：
*       绝对目录搜索
*       相对目录搜索
*如果最后一项是文件，则返回文件的名称
*/
private struct_mem_inode *get_dir(char *path,cstr filename,u8 *isfile)
{
    struct_mem_inode *inode,*itemp;
    struct_mem_buffer *mbuffer;
    struct_dir_entry *dir;
    char name[NAME_LEN];
    int i,dev,nr;
    
    /*如果进程根目录为空或工作目录为空，则说明内核有问题，因此出错调试*/
    if(!current->root || !current->root->count)
    {
        halts("no root inode.\n");
    }
    if(!current->pwd || !current->pwd->count)
    {
        halts("no process work directory.\n");
    }
    
    /*如果当前目录是从绝对目录开始*/
    if(path[0] == '/')
    {
        inode = current->root;
        /*指向下一目录开始处*/
        path ++;
    }
    /*如果不是，则是相对目录*/
    else if(path[0])
    {
        inode = current->pwd;
    }
    /*否则，返回空值*/
    else
    {
        return NULL;
    }
    
    /*inode引用计数加1*/
    inode->count ++;
    /*循环直到找到指定的目录*/
    while(TRUE)
    {
        /*如果到这里不是目录，则返回空值，说明没有找到*/
        if(!IS_DIR(inode->mode))
        {
            put_inode(inode);
            return NULL;
        }
        
        /*
        *如果剩下路径的长度为零，则上一个i节点就是所要找的节点
        *并且上一个i节点是目录，目录为：/cs/bin/
        */
        if(strlen(path) == 0)
        {
            *isfile = FALSE;
            return inode;
        }
        /*如果剩下的是文件名，或者是最后的目录，或者*/
        if((i == strchr(path,'/',strlen(path))) == -1)
        {
            /*如果找不到最后的目录项，则出错返回空值*/
            if(!(dir = find_entry(inode,path,&mbuffer)))
            {
                iput(inode);
                if(mbuffer)brelease(mbuffer);
                return NULL;
            }
            
            /*取得其i节点*/
            if(!(itemp = get_inode(inode->dev,dir->inode)))
            {
                iput(inode);
                if(mbuffer)brelease(mbuffer);
                return NULL;
            }
            
            /*判断最后一项是目录还是普通文件*/
            if(IS_DIR(itemp->mode))
            {
                *isfile = FALSE;
                if(mbuffer)
                {
                    brelease(mbuffer);
                }
                return itemp;
            }
            else
            {
                *isfile = TRUE;
                if(mbuffer)
                {
                    brelease(mbuffer);
                }
                strncpy(filename,path,strlen(name));
                return inode;
            }
        }
        
         /*获取目录名称*/
        if((i = strchr(path,'/',strlen(path))) >= 0)
        {
            strncpy(name,path,i + 1);
            name[i + 1] = 0;
            path += (i + 1);
        }
        
        if(!(dir = find_entry(inode,name,&mbuffer)))
        {
            put_inode(inode);
            return NULL;
        }
        
        dev = inode->dev;
        nr = dir->inode;
        brelease(mbuffer);
        iput(inode);
        
        /*取下一个目录的i节点内容*/
        if(!(inode = get_inode(dev,nr)))
        {
            /*不成功，则返回NULL表示失败*/
            return NULL;
        }
    }
}

/*返回指定目录名的i节点指针，以及最末端目录*/
private struct_mem_inode *dir_namei(char *path,
    cstr filename,u8 *isfile)
{
    
    return get_dir(path,filename,isfile);
}

/*
*获取指定路径的i节点，不论路径末尾是文件还是目录
*pathname指路径名，不论是目录路径或文件路径
*/
public struct_mem_inode *namei(char *pathname)
{
    struct_mem_inode *inode;
    struct_mem_buffer *mbuffer;
    struct_dir_entry *de;
    char filename[NAME_LEN];
    u8 isfile;
    int dev,nr;
    
    /*首先判断最末端是目录还是文件，如果是目录，直接返回*/
    if(!(inode = dir_namei(pathname,filename,&isfile)))
    {
        return NULL;
    }
    
    /*如果是目录，则直接返回*/
    if(!isfile)
    {
        return inode;
    }
    
    /*如果是文件，则从上面找到的目录中查找文件的i节点*/
    if(!(de = find_entry(inode,filename,&mbuffer)))
    {
        brelease(mbuffer);
        put_inode(inode);
        return NULL;
    }
    
    /*先释放，再使用*/
    dev = inode->dev;
    nr = de->inode;
    brelease(mbuffer);
    put_inode(inode);
    /*获取文件节点*/
    if((inode = get_inode(dev,nr)))
    {
        inode->last_access = CURRENT_TIME;
        inode->dirty = TRUE;
    }
    
    return inode;
}
