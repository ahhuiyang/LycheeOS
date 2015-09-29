/***************************************************************************
 *
 *  vsprintf.c-主要实现vsprintf函数
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 /*不能使用库中的函数，所以自己实现*/
#include "stdarg.h"
#include "string.h"
#include "type.h"

/*判断是不是数字字符*/
#define is_digit(c)     ((c) >= '0' && (c) <= '9')

/*用于数字和字符串格式化的标志*/
#define ZEROPAD     1       /*填充零*/
#define SIGN        2       /*是否带符号*/
#define PLUS        4       /*是否显示'+'字符，如果是正数*/
#define SPACE       8       /*如果显示加号，则置空格*/
#define LEFT        16      /*左对齐，默认是右对齐*/
#define SMALL       32      /*对于16进制来说，使用小写字母*/   

/*将数字字符串转换成数字，注意这里会改变s指向的字符串指针的位置，直到碰到非数字字符*/
private int atoi(const char **s)
{
    int ret = 0;
    
    while(is_digit(**s))
        ret = 10 * ret + (*((*s) ++) - '0');
        
    return ret; 
}

private int do_div(int *n,int base)
{
    *n = *n / base;
    return (*n % base); 
}

/*将数字字符串转换成指定格式的字符串*/
private char *number(char *str,int num,int base,int size,int precision,int flags)
{
    char c,sign,temp[1024];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;
    
    if(flags & SMALL)digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    if(flags & LEFT)flags &= ~ZEROPAD;
    if(base<2 || base >36)return str;
    
    c = (flags & ZEROPAD)?'0':CHAR_SPACE;
    if((flags & SIGN) && num < 0)
    {
        sign = '-';
        num = -num;
    }
    else
    {
        sign = (flags & PLUS)?'+':((flags & SPACE)?CHAR_SPACE:0);
    }
    
    if(sign)size--;
    
    /*如果是十六进程，开头要显示0x，如果是八进制，开头放一个零*/
    if(base == 16)
    {
        size -=2;
    }
    else if(base == 8)
    {
        size -=1;
    }
    
    i = 0;
    
    if(num != 0)
    {
        while(num != 0)
        {
            temp[i ++] = digits[do_div(&num,base)];
        }
    }
    else
    {
        temp[i ++] = 0;
    }
    
    /*不能丢失数据，若字符数大于精度值，则扩展精度值至数字个数*/
    if(i > precision)precision = i;
    size -= precision;
    
    /*下面开始真正形成字符串*/
    /*如果没有左标志和填零标志，则放入指定数目的空格*/
    if(!(flags & (ZEROPAD | LEFT)))
    {
        while(size -- > 0)
        {
            *(str ++) = CHAR_SPACE;
        }
    }
    /*如果有符号*/
    if(sign)*(str ++) = sign;
    /*如果是16位或8位，则加前缀*/
    if(base = 16)
    {
        *(str ++) = '0';
        *(str ++) = 'x';
    }
    else if(base = 8)
    {
        *(str ++) = '0';
    }
    /*如果没有左调整标志，则放入c字符，零或空格*/
    if(!(flags & LEFT))
    {
        while(size -- > 0)
        {
            *(str ++) = c;
        }
    }
    /*i中存放有数值num的数字个数，若数字个数小于精度值，则放入一定数量的'0'*/
    while(i < precision --)
    {
        * (str ++) = '0';
    }
    
    /*放入转换好的数字*/
    while(i -- > 0)
    {
        *(str ++) = temp[i];    /*temp中的数字字符高位在后低位在前*/
    }
    
    /*若有左靠齐标志，则补空格*/
    while((flags & LEFT) && size -- > 0)
    {
        *(str ++) = CHAR_SPACE;
    }
    
    return str;
}

/*
*   vsprintf:
*           char *buf:      经处理得到的字符串
*           char *fmt:      包含有占位符的格式字符串
*           va_list args:   指向参数列表中第一个参数
*
*   该函数返回转换好的字符串长度
*
*可识别的占位符包括%d,%md,%o,%x,%u,%c,%s,%ms,%-ms,%m.ns,%-m.ns,%f,%m.nf,%-m.nf,%p
*
*不支持浮点数
*/

public int vsprintf(char *buf,const char *fmt,va_list args)
{
    char *str;                  /*转换后的字符串保存位置，不能使用buf*/
    unsigned int flags;         /*用于数字转换的标志*/
    unsigned int field_width;   /*字段域长度*/
    unsigned int precision;     /*精度*/
    int i,len;
    char *str_temp;
    
    for(str = buf ; *fmt ; ++ fmt)
    {
        /*如果不是占位符标志，则直接存到结果字符串里*/
        if(*fmt != '%')
        {
            *(str ++) = *fmt;
            continue;
        }
        
        /*运行到这里，说明是占位符标志*/
        flags = 0;
        /*循环，直到所有的标志字符跳过*/
        while(1)
        {
            ++fmt;              /*skip the '%'*/
            switch(*fmt)
            {
                case '-':flags |= LEFT;break;
                case '+':flags |= PLUS;break;
                case ' ':flags |= SPACE;break;
                case '0':flags |= ZEROPAD;break;
            }
        }
        
        /*获得字段域长度*/
        field_width = -1;
        if(is_digit(*fmt))
            field_width = atoi(&fmt);   /*atoi会改变fmt的位置，使其正好指向下一位置，所以不用++fmt*/
        /*获取精度信息*/
        precision = -1;
        if(*fmt == '.')
        {
            ++fmt;
            precision = atoi(&fmt);
        }
        
        /*下面分析转换指示符*/
        switch(*fmt)
        {
            case 'c':
                if(!(flags & LEFT))
                    while(--field_width > 0)
                        *(str ++) = ' ';
                *(str ++) = (unsigned char)va_arg(args,int);
                /*如果没有左对齐标志，如果需要，右补空格*/
                /*这里是很有技巧性的，不能换成field_width--*/
                while(--field_width > 0)
                    *(str ++) = ' ';
                break;
            case 's':
                str_temp = va_arg(args,char *);
                len = strlen(str_temp);
                
                if(precision < 0)
                {
                    precision = len;
                }
                else if(len > precision)
                {
                    len = precision;
                }
                
                if(!(flags & LEFT))
                {
                    while(len < (field_width --))
                        *(str ++) = ' ';
                }
                for(i = 0 ; i < len ; i ++)
                {
                    *(str ++) = *(str_temp ++);
                }
                while(len < (field_width --))
                    *(str ++) = ' ';
                break;
            case 'o':
                str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
                break;
            case 'p':
                if(field_width = -1)
                {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str,(unsigned int)va_arg(args,void *),16,field_width,precision,flags);
                break;
            case 'x':
                flags |= SMALL;
            case 'X':
                str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
                break;
            case 'd':
                flags |= SIGN;
            case 'u':
                str = number(str,va_arg(args,unsigned int),10,field_width,precision,flags);
            default:
                if(*fmt)
                {
                    *(str ++) = *fmt;
                }
                else    /*说明已到字符串末尾，将fmt前移，这样循环判断*fmt = 0，就会退出循环*/
                {
                    --fmt;
                }
        }
    }
    
    *str = '\0';
    return str - buf;   /*返回转换后的字符串长度*/
}

