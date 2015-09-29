/***************************************************************************
 *
 *  process.c-操作进程
 *  Copyright (C) 2010 杨习辉
 ***************************************************************************/
 
#include "kernel.h"
#include "process.h"

public struct_process *process[NR_PROCESS];
