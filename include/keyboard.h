/***************************************************************************
 *
 *  keyboard.h-有关键盘的常量定义
 *  Copyright (C) 2010 杨习辉
 **************************************************************************/
 
 #ifndef _KEYBOARD_H
 #define _KEYBOARD_H
 
 /*有多少个键盘扫描码*/
 #define NR_SCAN_CODES  0x80
 #define NR_NUMPAD_KEY  0x10  
/*每一个ASCII对应多少种显示方式*/
 #define KEYMAP_COLS   3
 /*键盘缓冲区大小*/
 #define KEYBOARD_BUFFER_SIZE   32
 
 /*定义此标识的键都不是可写字符*/
 #define KEY_EXT         0x80      /*Normal function keys*/
/*Shift、Ctrl、Alt的位掩码*/
#define KEYMASK_EXT         0x80
#define KEYMASK_PRINT_CHAR  0x7F
#define KEYMASK_KEY         0xFF
#define KEYMASK_BREAK       0x80
#define KEYMASK_SHIFT_L     0x0200
#define KEYMASK_SHIFT_R     0x0400
#define KEYMASK_CTRL_L      0x0800
#define KEYMASK_CTRL_R      0x1000
#define KEYMASK_ALT_L       0x2000
#define KEYMASK_ALT_R       0x4000
/*小键盘按键掩码*/
#define KEYMASK_PAD         0x8000
/*Pause/Break键和PrintScreen键的位掩码*/
#define KEYMASK_PAUSEBREAK          0x80000000
#define KEYMASK_PRINTSCREEN         0x40000000
#define KEYMASK_NUMLOCK             0x20000000
#define KEYMASK_MAKED               0x10000000
#define KEYMASK_CAPS_LOCK           0x08000000
#define KEYMASK_SCROLL_LOCK         0x04000000

/*以下是不能显示的按键，*/
#define ESC         (0x01 + KEY_EXT)
#define TAB         (0x02 + KEY_EXT)
#define ENTER       (0x03 + KEY_EXT)
#define BACKSPACE   (0x04 + KEY_EXT)

#define GUI_L       (0x05 + KEY_EXT)
#define GUI_R       (0x06 + KEY_EXT)
#define APPS        (0x07 + KEY_EXT)

/*Shift、Ctrl、Alt*/
#define SHIFT_L     (0x08 + KEY_EXT)
#define SHIFT_R     (0x09 + KEY_EXT)
#define CTRL_L      (0x0A + KEY_EXT)
#define CTRL_R      (0x0B + KEY_EXT)
#define ALT_L       (0x0C + KEY_EXT)
#define ALT_R       (0x0D + KEY_EXT)

/*lock*/
#define CAPS_LOCK   (0x0E + KEY_EXT)
#define NUM_LOCK    (0x0F + KEY_EXT)
#define SCROLL_LOCK (0x10 + KEY_EXT)

/*fucntion keys*/
#define F1          (0x11 + KEY_EXT)
#define F2          (0x12 + KEY_EXT)
#define F3          (0x13 + KEY_EXT)
#define F4          (0x14 + KEY_EXT)
#define F5          (0x15 + KEY_EXT)
#define F6          (0x16 + KEY_EXT)
#define F7          (0x17 + KEY_EXT)
#define F8          (0x18 + KEY_EXT)
#define F9          (0x19 + KEY_EXT)
#define F10         (0x1A + KEY_EXT)
#define F11         (0x1B + KEY_EXT)
#define F12         (0x1C + KEY_EXT)

/*control pad keys*/
#define PRINTSCREEN (0x1D + KEY_EXT)
#define PAUSEBREAK  (0x1E + KEY_EXT)
#define INSERT      (0x1F + KEY_EXT)
#define DELETE      (0x20 + KEY_EXT)
#define HOME        (0x21 + KEY_EXT)
#define END         (0x22 + KEY_EXT)
#define PAGEUP      (0x23 + KEY_EXT)
#define PAGEDOWN    (0x24 + KEY_EXT)
#define UPARROW     (0x25 + KEY_EXT)
#define DOWNARROW   (0x26 + KEY_EXT)
#define LEFTARROW   (0x27 + KEY_EXT)
#define RIGHTARROW  (0x28 + KEY_EXT)

/*acpi keys*/
#define POWER       (0x29 + KEY_EXT)
#define SLEEP       (0x2A + KEY_EXT)
#define WAKE        (0x2B + KEY_EXT)

/*num pad keys*/
#define NUMPAD_SLASH    (0x2C + KEY_EXT)
#define NUMPAD_STAR     (0x2D + KEY_EXT)
#define NUMPAD_MINUS    (0x2E + KEY_EXT)
#define NUMPAD_PLUS     (0x2F + KEY_EXT)
#define NUMPAD_ENTER    (0x30 + KEY_EXT)
#define NUMPAD_DOT      (0x31 + KEY_EXT)
#define NUMPAD_0        (0x32 + KEY_EXT)
#define NUMPAD_1        (0x33 + KEY_EXT)
#define NUMPAD_2        (0x34 + KEY_EXT)
#define NUMPAD_3        (0x35 + KEY_EXT)
#define NUMPAD_4        (0x36 + KEY_EXT)
#define NUMPAD_5        (0x37 + KEY_EXT)
#define NUMPAD_6        (0x38 + KEY_EXT)
#define NUMPAD_7        (0x39 + KEY_EXT)
#define NUMPAD_8        (0x3A + KEY_EXT)
#define NUMPAD_9        (0x3B + KEY_EXT)

#define NUMPAD_UP       NUMPAD_8
#define NUMPAD_DOWN     NUMPAD_2
#define NUMPAD_LEFT     NUMPAD_4
#define NUMPAD_RIGHT    NUMPAD_6
#define NUMPAD_HOME     NUMPAD_7
#define NUMPAD_END      NUMPAD_1
#define NUMPAD_PAGEUP   NUMPAD_9
#define NUMPAD_PAGEDOWN NUMPAD_3
#define NUMPAD_INS      NUMPAD_0
#define NUMPAD_MID      NUMPAD_5
#define NUMPAD_DEL      NUMPAD_DOT

/*键盘的数据寄存器和控制寄存器*/
#define KEYBOARD_CTL    0x64
#define KEYBOARD_DATA   0x60

/*原始数据缓冲区*/
typedef struct _struct_keyboard
{
    char* p_head;
    char* p_tail;
    unsigned int count;
    char buffer[KEYBOARD_BUFFER_SIZE];
}struct_keyboard;
#endif
