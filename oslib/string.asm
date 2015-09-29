;***************************************************************************
;
;   string.asm-内存及字符串操作函数，应使用nasm编译
;   Copyright (C) 2010 杨习辉
 ;**************************************************************************
 
global memcpy,memset,strlen,strchr
global set_bit,clear_bit,find_first_zero
 
 ;函数调用压栈图：
 ;|-----------
 ;  形参
 ;|-----------
 ; 返回地址
 ;|-----------
 ;ebp
 ;|-----------
 
;char* memcpy(char* es:dst,char* ds:src,unsigned int count)
;es和ds必须被设置好
;----------------------------------------------------------------
memcpy:
    push    ebp
    mov     ebp,esp
    
    push    esi
    push    edi
    push    ecx
    
    mov     esi,[ebp + 8]   ;dst
    mov     edi,[ebp + 12]  ;src
    mov     ecx,[ebp + 16]  ;size
    
    cmp     ecx,0           ;判断ecx是否大于0
    jle     label_memcpy_exit
    
    rep movsb
    
label_memcpy_exit:
    mov     eax,[ebp + 8]   ;dst,返回值    
    
    pop     ecx
    pop     edi
    pop     esi
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;void *memset(void *s,char c,unsigned int count)
;----------------------------------------------------------------
memset:
    push    ebp
    mov     ebp,esp
    
    mov     edi,[ebp + 8]   ;s
    mov     eax,[ebp + 12]  ;c
    and     eax,0xFF
    mov     ecx,[ebp + 16]  ;count
    
    rep stosb               ;al->es:di until cx = 0
    
    mov     eax,[ebp + 8]   ;s,返回值
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;unsigned int strlen(char *str)，不包含末尾的0
;----------------------------------------------------------------
strlen:
    push    ebp
    mov     ebp,esp
    
    xor     eax,eax         ;存放返回值
    mov     edi,[ebp + 8]   ;s
.again:
    cmp     dword [edi],0
    je      label_strlen_exit
    inc     eax
    inc     edi
    loop    .again
    
label_strlen_exit:
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;unsigned int strchr(char *str,char c,int len)
;从0开始的位置，返回－1表示失败
;----------------------------------------------------------------
strchr:
    push    ebp
    mov     ebp,esp
    
    push    ebx
    push    ecx
    
    xor     eax,eax
    xor     ecx,ecx
    xor     edx,edx
    mov     ebx,[ebp + 8]       ;str
    mov     al,[ebp + 12]       ;for scasb
    mov     ecx,[ebp + 16]      ;for scasb count
    
    cld
    repe    scasb
    
    mov     eax,-1
    jne     label_exit
    mov     eax,[ebp + 16]      ;len
    sub     eax,ecx             ;len-ecx
    dec     eax                 ;len-ecx-1
    
    pop     ecx
    pop     ebx
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;int set_bit(void *addr,unsigned int nr)
;设置比特位，返回源比特位，nr可以大于32
;----------------------------------------------------------------
set_bit:
    push    ebp
    mov     ebp,esp
    
    push    ebx
    push    edx
    
    xor     ebx,ebx
    xor     eax,eax
    mov     edx,[ebp + 8]       ;addr
    mov     ebx,[ebp + 12]      ;nr
    bts     [edx],ebx          ;设置[addr]中由ebx指定的位
    setb    al                  ;set byte if cf = 1，这也是返回值
    
    pop     edx
    pop     ebx
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;int clear_bit(void *addr,unsigned int nr)
;清除比特位，返回原比特位，nr可以大于32
;----------------------------------------------------------------
clear_bit:
    push    ebp
    mov     ebp,esp
    
    push    ebx
    push    edx
    
    xor     ebx,ebx
    xor     eax,eax
    mov     edx,[ebp + 8]       ;addr
    mov     ebx,[ebp + 12]      ;nr
    btr     [edx],ebx          ;清除[addr]中由ebx指定的位
    setb    al                  ;set byte if cf = 1，这也是返回值
    
    pop     edx
    pop     ebx
    mov     esp,ebp
    pop     ebp
    
    ret
;----------------------------------------------------------------
;int find_first_zero(void *addr)
;在一个数据块（1024字节，8192位）查找第一个是零的位的偏移
;----------------------------------------------------------------
find_first_zero:
    push    ebp
    mov     ebp,esp
    
    push    eax
    push    ecx
    push    edx
    push    esi
    
    cld
    xor     ecx,ecx
    xor     edx,edx
    mov     esi,[ebp + 8]       ;addr
label_research:
    lodsd                       ;取ds:esi -> eax，ds:esi + 4
    not     eax                 ;取反
    bsf     edx,eax
    je      not_found           ;如果eax是全零，则ZF=1，表明没找到，跳转继续找
    add     ecx,edx             ;ecx中存储偏移值
    jmp     label_exit
not_found:
    add     ecx,32
    cmp     ecx,8192
    jb      label_research

label_exit:
    mov     eax,ecx             ;返回值，如果成果，返回结果小于8192，否则失败
    pop     esi
    pop     edx
    pop     ecx
    pop     eax
    mov     esp,ebp
    pop     ebp
    
    ret
