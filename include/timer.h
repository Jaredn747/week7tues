#ifndef __TIMER_H__
#define __TIMER_H__

#include "defines.h"

#define TLOAD 0x0 // reload register; use BGLOAD
#define TVALUE 0x1 // current count value
#define TCNTL 0x2 // control register
#define TINTCLR 0x3 // write to this R clears timer interrupt
#define TRIS 0x4 // raw interrupt status

#define TMIS 0x5 // masked interuutp status
#define TBGLOAD 0x6 // count down and reload value

typedef volatile struct timer {
    u32 *base; // timer's base address; as u32 pointer
    int tick, hh, mm, ss; // per timer data area
    char clock[16];
}   TIMER;

extern void timer_handler();
extern volatile TIMER timer[4]; // 4 TIMER structure pointers
extern void timer_init();
extern void timer_start(int n);
extern int timer_clearInterrupt(int n);

#endif