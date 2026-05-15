#ifndef __KBD_H__
#define __KBD_H__

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER 0x5A
#define LCTRL 0x14
#define RCTRL 0xE014
#define ESC 0x76
#define F1 0x05
#define F2 0x06
#define F3 0x04
#define F4 0x0C
#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK 0x0C
#define KISTA 0x10

typedef volatile struct kbd {  // base = 0x10006000
  char* base;                  // base address of KBD, as char *
  char buf[128];               // input buffer
  int head, tail, data, room;  // control variables
} KBD;

extern volatile KBD kbd;  // KBD data structure

extern int kbd_init();
extern void kbd_handler();
extern int kgetc();
extern int kgets(char s[]);

#endif