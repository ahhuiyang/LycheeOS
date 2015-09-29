#----------------------------------------------------------------------
#
#Makefile for LycheeOS
#
#copyright (C) 2010 ÑîÏ°»Ô
#----------------------------------------------------------------------

#asm and c complier,linker and their flags
ASM         = nasm
CC          = gcc
LD          = ld

#flags
ASMFLAGS    = -f elf -i include/
CCFLAGS     = -c -fno-builtin -nostdinc -I include/
LDFLAGS     = -r -M

#complie

all:boot/oshead.o oslib/oslib.o mm/mm.o \
	fs/fs.o kernel/blk/blk.o kernel/chr/tty.o \
	kernel/kernel.o system.o
#boot
boot/oshead.o:boot/loader.asm boot/rmtopm.asm boot/oshead.asm
	$(ASM) boot/loader.asm -o boot/loader.bin -l boot/loader.lst
	$(ASM) $(ASMFLAGS) boot/rmtopm.asm -o boot/rmtopm.bin -l boot/rmtopm.lst
	$(ASM) $(ASMFLAGS) boot/oshead.asm -o boot/oshead.o -l boot/oshead.lst

#oslib
oslib/oslib.o:oslib/oslib.asm oslib/string.asm oslib/clib.c
	$(ASM) $(ASMFLAGS) oslib/oslib.asm -o oslib/osliba.o -l oslib/osliba.lst
	$(ASM) $(ASMFLAGS) oslib/string.asm -o oslib/string.o -l oslib/string.lst
	$(CC) $(CCFLAGS) oslib/clib.c -o oslib/clib.o
	$(LD) $(LDFLAGS) oslib/osliba.o oslib/string.o oslib/clib.o -o oslib/oslib.o>oslib/oslib.map
    
#mm
mm/mm.o:mm/pf.asm mm/memory.c
	$(ASM) $(ASMFLAGS) mm/pf.asm -o mm/pf.o -l mm/pf.lst
	$(CC) $(CCFLAGS) mm/memory.c -o mm/memory.o
	$(LD) $(LDFLAGS) mm/pf.o mm/memory.o -o mm/mm.o>mm/mm.map

#file system
fs/fs.o:fs/bitmap.c fs/super.c fs/inode.c fs/truncate.c \
        fs/namei.c fs/file.c fs/block_dev.c fs/chr_dev.c \
        fs/file_dev.c fs/rw.c
	$(CC) $(CCFLAGS) fs/bitmap.c -o fs/bitmap.o
	$(CC) $(CCFLAGS) fs/super.c -o fs/super.o
	$(CC) $(CCFLAGS) fs/inode.c -o fs/inode.o
	$(CC) $(CCFLAGS) fs/truncate.c -o fs/truncate.o
	$(CC) $(CCFLAGS) fs/namei.c -o fs/namei.o
	$(CC) $(CCFLAGS) fs/file.c -o fs/file.o
	$(CC) $(CCFLAGS) fs/block_dev.c -o fs/block_dev.o
	$(CC) $(CCFLAGS) fs/chr_dev.c -o fs/chr_dev.o
	$(CC) $(CCFLAGS) fs/file_dev.c -o fs/file_dev.o
	$(CC) $(CCFLAGS) fs/rw.c -o fs/rw.o
	$(LD) $(LDFLAGS) fs/bitmap.o fs/super.o \
        fs/inode.o fs/truncate.o fs/namei.o \
        fs/file.o fs/block_dev.o fs/chr_dev.o \
        fs/file_dev.o fs/rw.o -o fs/fs.o>fs/fs.map

#blk
kernel/blk/blk.o:kernel/blk/hd.c kernel/blk/rw_blk.c kernel/blk/blk.c
	$(CC) $(CCFLAGS) kernel/blk/hd.c -o kernel/blk/hd.o
	$(CC) $(CCFLAGS) kernel/blk/rw_blk.c -o kernel/blk/rw_blk.o
	$(CC) $(CCFLAGS) kernel/blk/blk.c -o kernel/blk/blkc.o
	$(LD) $(LDFLAGS) kernel/blk/hd.o kernel/blk/rw_blk.o \
        kernel/blk/blkc.o -o kernel/blk/blk.o>kernel/blk/blk.map
        
#chr
kernel/chr/tty.o:kernel/chr/keyboard.c kernel/chr/console.c kernel/chr/tty.c
	$(CC) $(CCFLAGS) kernel/chr/keyboard.c -o kernel/chr/keyboard.o
	$(CC) $(CCFLAGS) kernel/chr/console.c -o kernel/chr/console.o
	$(CC) $(CCFLAGS) kernel/chr/tty.c -o kernel/chr/ttyc.o
	$(LD) $(LDFLAGS) kernel/chr/keyboard.o kernel/chr/console.o \
        kernel/chr/ttyc.o -o kernel/chr/tty.o>kernel/chr/tty.map
        
#kernel
kernel/kernel.o:kernel/interrupt.asm kernel/interrupt.c kernel/i8259.asm \
        kernel/i8259.c kernel/syscall.asm kernel/syscall.c \
        kernel/buffer.c kernel/fork.c kernel/kernel.asm kernel/kernel.c \
        kernel/ktime.c kernel/vsprintf.c kernel/printfs.c kernel/sched.c \
        kernel/process.c
	$(ASM) $(ASMFLAGS) kernel/interrupt.asm -o kernel/interrupta.o -l kernel/interrupta.lst
	$(CC) $(CCFLAGS) kernel/interrupt.c -o kernel/interruptc.o
	$(LD) $(LDFLAGS) kernel/interrupta.o kernel/interruptc.o -o kernel/interrupt.o>kernel/interrupt.map
	$(ASM) $(ASMFLAGS) kernel/i8259.asm -o kernel/i8259a.o -l kernel/i8259a.lst
	$(CC) $(CCFLAGS) kernel/i8259.c -o kernel/i8259c.o
	$(LD) $(LDFLAGS) kernel/i8259a.o kernel/i8259c.o -o kernel/i8259.o>kernel/i8259.map    
	$(ASM) $(ASMFLAGS) kernel/syscall.asm -o kernel/syscalla.o -l kernel/syscalla.lst
	$(CC) $(CCFLAGS) kernel/syscall.c -o kernel/syscallc.o
	$(LD) $(LDFLAGS) kernel/syscalla.o kernel/syscallc.o -o kernel/syscall.o>kernel/syscall.map
	$(CC) $(CCFLAGS) kernel/buffer.c -o kernel/buffer.o
	$(CC) $(CCFLAGS) kernel/fork.c -o kernel/fork.o
	$(ASM) $(ASMFLAGS) kernel/kernel.asm -o kernel/kernela.o -l kernel/kernela.lst
	$(CC) $(CCFLAGS) kernel/kernel.c -o kernel/kernelc.o
	$(LD) $(LDFLAGS) kernel/kernela.o kernel/kernelc.o -o kernel/kernelt.o>kernel/kernelt.map
	$(CC) $(CCFLAGS) kernel/ktime.c -o kernel/ktime.o
	$(CC) $(CCFLAGS) kernel/vsprintf.c -o kernel/vsprintf.o
	$(CC) $(CCFLAGS) kernel/printfs.c -o kernel/printfs.o
	$(LD) $(LDFLAGS) kernel/vsprintf.o kernel/printfs.o -o kernel/printf.o>kernel/printf.map
	$(CC) $(CCFLAGS) kernel/sched.c -o kernel/sched.o
	$(CC) $(CCFLAGS) kernel/process.c -o kernel/process.o
	$(LD) $(LDFLAGS) kernel/interrupt.o kernel/i8259.o \
        kernel/syscall.o kernel/buffer.o kernel/fork.o \
        kernel/kernelt.o kernel/ktime.o kernel/printf.o \
        kernel/sched.o kernel/process.o -o kernel/kernel.o>kernel/kernel.map

init/kmain.o:init/kmain.c
	$(CC) $(CCFLAGS) init/kmain.c -o init/kmain.o
        
system.o:boot/oshead.o oslib/oslib.o mm/mm.o fs/fs.o \
        kernel/blk/blk.o kernel/chr/tty.o kernel/kernel.o
	$(LD) $(LDFLAGS) -r boot/oshead.o oslib/oslib.o mm/mm.o \
        fs/fs.o kernel/blk/blk.o kernel/chr/tty.o \
        kernel/kernel.o -o system.o>system.map
        
clean:
	rm -f boot/*.o boot/*.lst boot/*.map boot/*.bin \
			oslib/*.o oslib/*.lst oslib/*.map \
			mm/*.o mm/*.lst mm/*.map \
			fs/*.o fs/*.lst fs/*.map \
			kernel/blk/*.o kernel/blk/*.lst kernel/blk/*.map \
			kernel/chr/*.o kernel/chr/*.lst kernel/chr/*.map \
			kernel/*.o kernel/*.lst kernel/*.map \
			system.o system.map

