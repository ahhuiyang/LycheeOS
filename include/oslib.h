/***************************************************************************
 *
 *  oslib.h-内核程序库
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
#ifndef _OSLIB_H
#define _OSLIB_H

void enable_int(void);
void disable_int(void);
void nop(void);

#define MAX(a,b) (((a) > (b))?(a):(b))
#define MIN(a,b) (((a) > (b))?(b):(a))

#endif
