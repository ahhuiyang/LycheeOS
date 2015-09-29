/***************************************************************************
 *
 *  memory.c-内存管理主文件，定义了内存页、页表及页目录
 *  管理的相关函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#include "type.h"
#include "kernel.h"
#include "memory.h"
#include "string.h"

/*
*最大可分页内存量，现在定义最大为64MB
*以后可以通过修改下面的数值来自定义
*/
#define PAGE_MEMORY (63 * 1024 * 1024)
#define NR_PAGES    (PAGE_MEMORY >> 12)

/*
*标志一个内存页是否正在使用
*定义如此大数字的原因，是由于程序中会递减内存使用计数
*只有为0时，才能说明内存不在使用
*/
#define PAGE_USED   0x7F
#define PAGE_UNUSED 0

/*给定一个地址，求页面号，页面地址是物理地址*/
#define PAGE_NR(addr) ( ((addr) - main_memory_start) >> 12)

/*由线性地址求出对应页目录项的地址*/
#define PG_DIR(addr) (sys_pgdir + (((addr) >> 20) & 0xFFC))
#define PG_TBL(addr) ((0xFFFFF000 & *(u32 *)PG_DIR(addr)) + ((addr >> 10) & 0xFFC))
/*分离出线性地址中的索引*/
#define ADDR_PG_DIR(addr) (((addr) >> 22) & 0x3FF)
#define ADDR_PG_TBL(addr) (((addr) >> 12) & 0x3FF)

/*可用的内存页面*/
private int main_memory_pages;

/*页映射数组，每1字节代码一页内存*/
private unsigned char mem_map[NR_PAGES];

/*
*get_free_page：
*       取第一个空闲页面，并标志为已使用。如果没有空闲页面，就返回0
*       这个函数只取得物理地址，并不映射到进程的线性地址空间中去
*       返回页面的起始地址
*       这个函数操作的是物理页面
*/
public int get_phy_page(void)
{
    int i,phy;
    
    /*循环查找第一个为空的内存页面*/
    for(i = 0 ; i < main_memory_pages ; i ++)
    {
        if(!mem_map[i])
        {
            break;
        }
    }
    
    /*如果没有可用内存，则返回0*/
    if(i >= main_memory_pages)return 0;
    
    /*计算物理地址*/
    phy = main_memory_start + i * PAGE_SIZE;
    
    /*清内存*/
    memset((char *)phy,0,PAGE_SIZE);
    
    /*返回申请到的物理地址*/
    return phy;
}

/*
*释放物理地址addr开始的一页内存
*如果内存在系统空间、缓冲空间或超出物理地址空间，则返回
*参数addr是物理地址
*/
public void free_phy_page(unsigned int addr)
{
    int i;
    
    if(addr < main_memory_start
        || addr > (main_memory_end - PAGE_SIZE))
    {
        return;
    }
    
    /*计算内存页号*/
    i = PAGE_NR(addr);
    /*如果内存已经为空，则不进行操作*/
    if(!mem_map[0])
    {
        return;
    }
    
    /*将引用计数减1，这样如果有几个进程映射此页，还会反应出其映射次数*/
    mem_map[i] --;
}

/*
*释放指定内存块及管理这块内存的页表
*每一个页目录项指向一个页表，每一个页表可管理4MB的内存
*参数：
*       addr:要释放的线性内存起始地址
*       size:内存块的长度，以字节为单位
*操作对象是线性地址
*/
public void free_page_tables(u32 addr,u32 size)
{
    u32 *pg_table;
    u32 *pg_dir;
    int pdes,ptes;
    int i;
    
    /*首先检查addr是否在4MB边界上，否则不予处理*/
    if(addr & ((1 << 22) - 1))
    {
        return;
    }
    
    /*不能在系统内存空间区域或高速缓冲区*/
    if((addr >= sys_memory_start && addr < sys_memory_end)
        || (addr >= buffer_memory_start && addr < buffer_memory_end))
    {
        return;
    }
    
    /*计算size给出的内存大小所占用的目录项数，方法是除以4MB*/
    pdes = (size + ((1 << 22) - 1)) >> 22;
    
    /*计算起始页目录项的地址*/
    pg_dir = (u32 *)(sys_pgdir + ((addr >> 20) & 0x0FFC));
    
    /*循环释放页面*/
    for( ; size > 0 ; size --)
    {
        /*如果当前页目录指示页表不在内存中，则继续下一个页表*/
        if(!(PD_P & *pg_dir))
        {
            continue;
        }
        
        /*取页表地址*/
        pg_table = (u32 *)(0xFFFFF000 & *pg_dir);
        
        /*循环释放每个页表项所指的页面*/
        for(i = 0 ; i < 1024 ; i ++)
        {
            /*若页面表项所指的内存页面在内存中，则释放它*/
            if(PT_P & *pg_table)
            {
                free_phy_page(0xFFFFF000 & *pg_table);
            }
            
            /*页表项清空*/
            *pg_table = 0;
            
            /*指向下一表项*/
            pg_table ++;
        }
        
        /*释放页表所占用的页面*/
        free_page(0xFFFFF000 & *pg_dir);
        
        /*对应页目录项的内容为空*/
        *pg_dir = 0;
        
        /*指向下一个要处理的页目录项*/
        pg_dir ++;
    }
    
    /*
    *由于页目录和页表的内容改变了，因此TLB中会与实际不一致
    *重新加载CR3，可使TLB中的内容无效
    */
    reload_cr3();
}

/*
*复制页目录项和页表项
*复制指定线性地址和长度内存对应的页目录项和页表项
*从而使相应物理内存被两个页表所映射而共享使用
*复制时，需要申请页面来存放新页表
*参数：
*   from-源线性地址，必须是4MB边界对齐
*   to  -目的线性地址，必须是4MB边界对齐
*此函数操作的是线性地址
*/
public boolean copy_page_tables(u32 from,u32 to,int size)
{
    u32 *from_pt,*to_pt;
    u32 *from_pd,*to_pd;
    u32 page;
    int i,count;
    
    /*现在判断from和to必须是4MB边界对齐，如果不是，则退出*/
    if((from & 0x3FFFFF) || (to & 0x3FFFFF))
    {
        return FALSE;
    }
    
    /*求出源线性地址和目的线性地址对应的目录项*/
    from_pd = (u32 *)((from >> 20) & 0xFFC);
    to_pd = (u32 *)((to >> 20) & 0xFFC);
    
    /*要复制的页表数*/
    count = ((u32)(size + 0x3FFFFF)) >> 22;
    
    /*循环复制页表*/
    for( ; count > 0 ; count --)
    {
        /*如果目标目录项所指向的页表已存在，则说明内核错误*/
        if(PD_P & *to_pd)
        {
            halts("copy_page_tables-page table already exists.\n");
        }
        
        /*如果源目录项所指向的页表不存在，则跳过*/
        if(!(PD_P & *from_pd))
        {
            continue;
        }
        
        /*取得源页表地址*/
        from_pt = (u32 *)(0xFFFFF000 & *from_pd);
        
        /*为新的目的页表申请一页物理内存*/
        if(!(to_pt = (u32 *)get_phy_page()))
        {
            /*没有申请到一页物理内存，则返回*/
            return FALSE;
        }
        
        /*
        *设置目录项的值为申请到的页表地址
        *申请的页表地址最低12位是0，也就是说是以4KB边界对齐的
        *新页目录项中一些位的设定：
        *       p位：1，说明页表在内存中，页目录有效
        *       r/w:1，说明写读可写
        *       u/s:1，说明是用户级页表
        */
        *to_pd = ((u32)to_pt & 0xFFFFF000) | 7;
        
        /*
        *要复制多少个页表项
        *对于系统页表项，由于系统只在前640KB部分活动
        *并且其它进程不会映射到从0开始的地址空间，所以只需复制640KB部分
        */
        i = (from == 0)?160:1024;
        
        /*循环复制页表项*/
        for( ; i > 0 ; i --)
        {
            page = *from_pt;
            
            /*如果源页表项所指向的内存页面不在内存，则继续下一项*/
            if(!(PT_P & page))
            {
                continue;
            }
            
            /*
            *将页表项指向的内存页面只读，这样就会产生写时复制了
            *因为复制过后，源页表项和目的页表项指向的是同一物理内存页面
            */
            page &= (~2);
            
            /*拷贝到目的页表项*/
            *to_pt = page;
            
            /*
            *注意，如果页表项所指向的物理内存在主存储区
            *则要同时设置源页表项和目标页表项所指向的内存为“只读”
            *这样不论是源进程还是目标进程，当对共享内存进行操作时，都会发生“写时复制”
            *对于内核空间则不然，内核代码可以任意更改内存，而不应该发生写时复制
            */
            if((page & 0xFFFFF000) >= main_memory_start)
            {
                *from_pt &= (~2);
                /*引用计数加1*/
                mem_map[PAGE_NR(page)] ++;
            }
            
            /*指向下一个页表 项*/
            from_pt ++;
            to_pt ++;
        }
    }
    
    /*使TLB无效*/
    reload_cr3();
    
    return TRUE;
}

/*
*将一物理内存页面放在指定的线性址址处
*参数：
*       phy_page-物理内存页面起始地址
*       address-线性地址起始位置
*这个函数实际要做的工作就是设置相应的页目录和页表
*使线性地址映射到指定的物理内存上
*此函数返回物理地址
*/
public u32 map_page(u32 phy_page,u32 address)
{
    u32 page,*pt;
    
    /*物理内存页必须是主存区域*/
    if(page < main_memory_start && page >= main_memory_end)
    {
        return 0;
    }
    
    /*用于映射的内存引用次数必须是1，也就是说必须是已经申请的页面*/
    if(mem_map[PAGE_NR(phy_page)] != 1)
    {
        return 0;
    }
    
    /*取得所在的页目录地址*/
    pt = (u32 *)(PG_DIR(address));
    
    /*
    *如果页目录所指向的页表有效，则取得页表项的地址
    *否则，新申请一个页面
    */
    if((*pt) & PD_P)
    {
        pt = (u32 *)(0xFFFFF000 & *pt);
    }
    else
    {
        if(!(page = get_phy_page()))
        {
            return 0;
        }
        
        /*设置页目录项的属性为存内存、可读可写和用户级别*/
        *pt = page | 7;
        /*然后再取得新页表的首地址*/
        pt = (u32 *)page;
    }
    
    /*最后，在页表相应位置找到相应页表项，让其它向这一页物理内存，并设置属性*/
    pt[ADDR_PG_TBL(address)] = phy_page | 7;
    
    /*如果返回的是物理地址，则说明操作成功*/
    return page;
}

/*
*异常处理函数，处理写时复制
*写时复制发生在当CPU试图写入只读内存时
*当发生写时复制时，根据异常发生时的线性地址
*系统新申请一页内存，并将线性地址映射到这块内存上
*参数由#PF页异常处理函数压入，含义如下：
*   err_code-只有低3位有效，分别是：
*       bit0:P位，异常原因。0-页面不存在，1-违反页级保护权限
*       bit1:W/R位，异常原因。0-读引起，1-写引起
*       bit2:U/S位，发生异常时CPU执行的代码级别。
*            U/S=0，表示CPU正在执行超级用户代码
*            U/S=1，表示CPU正在执行一般用户代码
*   address-发生页异常时CPU试图进行操作的线性地址
*/
public void do_wp_page(u32 err_code,u32 address)
{
    u32 *pte;
    u32 old_page,new_page;
    
    /*计算线性地址对应的页表项*/
    pte = (u32 *)PG_TBL(address);
    
    /*取源物理页面地址*/
    old_page = 0xFFFFF000 & *pte;
    
    /*如果页面未被共享，则*/
    if(old_page >= main_memory_start && mem_map[PAGE_NR(old_page)] == 1)
    {
        *pte |= 2;
        reload_cr3();
        return;
    }
    
    /*为新页面申请一页内存*/
    if(!(new_page = get_phy_page()))
    {
        printfs("out of memory.\n");
        return;
    }
    
    /*如果源物理页表在主存区中，则内存映射引用计数减1*/
    if(old_page >= main_memory_start)
    {
        mem_map[PAGE_NR(old_page)] --;
    }
    
    /*重新设置页表项内容，置为在内存，可读写，用户级别*/
    *pte = (new_page | 7);
    
    /*复制内存，内核可以管理所有内存*/
    memcpy((char *)new_page,(char *)old_page,4096);
    
    /*刷新TLB*/
    reload_cr3();
}

/*
*缺页异常处理函数
*当访问不存在的页面时调用的函数
*/
public void do_no_page(u32 err_code,u32 address)
{
    return;
}

/*
*写页面验证
*若页面不可写，则复制页面，也就是“写时复制”
*该函数操作的是线性地址
*/
public void write_verify(u32 address)
{
    u32 page;
    
    /*若对应的页目录项不存在，则退出操作*/
    if(!((page = PG_DIR(address)) & PD_P))
    {
        return;
    }
    
    /*取得页表项地址*/
    page &= 0xFFFFF000;
    page += ((address >> 10) & 0xFFC);
    
    /*如果页表项指示，该页表项所指的物理页面有效，但只读，则执行“写时复制”*/
    if((PT_P | PT_RW) & *(u32 *)page == PT_P)
    {
        do_wp_page(0,page);
    }
    
    return;
}

/*
*取得页空闲物理内存并映射到指定的线性地址处
*与get_phy_page()不同的是，此函数不仅申请一页物理内存，而且将之映射到指定线性地址处
*该函数的参数是线性地址
*/
public u32 get_empty_page(u32 address)
{
    u32 page;
    
    /*首先取得一页物理内存*/
    if(!(page = get_phy_page()))
    {
        printfs("out of memory.\n");
        return 0;
    }
    
    /*然后映射到指定线性地址处*/
    if(!map_page(page,address))
    {
        free_phy_page(page);
        printfs("can not map physical page.\n");
        return 0;
    }
    
    return address;
}


/*内存管理初始化*/
public void mem_init()
{
    int i,count;
    
    /*初始化内存映射数组全部为已使用*/
    for(i = 0 ; i < NR_PAGES ; i ++)
    {
        mem_map[i] = PAGE_USED;
    }
    
    /*在主内存区中，有多个可用的内存页*/
    count = (main_memory_end - main_memory_start) >> 12;
    main_memory_pages = count;
    /*将mem_map中主内存区中对应的字节清零，表示没有使用*/
    for(i = 0 ; i < count ; i ++)
    {
        mem_map[i] = PAGE_UNUSED;
    }
}
