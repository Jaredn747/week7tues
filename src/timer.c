#include "timer.h"

#include "defines.h"
#include "vid.h"

// timer register u32 offsets from base address

volatile TIMER timer[4];  // 4 timers; 2 per unit; at 0x00 and 0x20

void timer_init() {
  int i;
  TIMER* tp;
  printf("timer_init()\n");
  for (i = 0; i < 4; i++) {
    tp = &timer[i];
    if (i == 0) tp->base = (u32*)0x101E2000;
    if (i == 1) tp->base = (u32*)0x101E2020;
    if (i == 2) tp->base = (u32*)0x101E3000;
    if (i == 3) tp->base = (u32*)0x101E3020;
    *(tp->base + TLOAD) = 0x0;          // reset
    *(tp->base + TVALUE) = 0xFFFFFFFF;  // read only; can be any value
    *(tp->base + TRIS) = 0x0;           // status
    *(tp->base + TMIS) = 0x0;           // status
    *(tp->base + TLOAD) = 0x100;
    // CntlReg=011-0110=|En|Pe|IntE|-|scal=01|32bit|0=wrap|=0x66
    *(tp->base + TCNTL) = 0x66;
    *(tp->base + TBGLOAD) = 0x1C00;           // timer counter value
    tp->tick = tp->hh = tp->mm = tp->ss = 0;  // initialize wall clock
    strcpy((char*)tp->clock, "00:00:00");
  }
}

void timer_handler(int n) {
  int i;
  TIMER* t = &timer[n];
  t->tick++;  // Assume 120 ticks per second
  if (t->tick == 12) {
    t->tick = 0;
    t->ss++;
    if (t->ss == 60) {
      t->ss = 0;
      t->mm++;
      if (t->mm == 60) {
        t->mm = 0;
        t->hh++;  // no 24 hour roll around
      }
    }
    t->clock[7] = '0' + (t->ss % 10);
    t->clock[6] = '0' + (t->ss / 10);
    t->clock[4] = '0' + (t->mm % 10);
    t->clock[3] = '0' + (t->mm / 10);
    t->clock[1] = '0' + (t->hh % 10);
    t->clock[0] = '0' + (t->hh / 10);

    color = n;  // display in different color
    for (i = 0; i < 8; i++) {
      unkpchar(219, n, 70 + i);
      kpchar(t->clock[i], n, 70 + i);  // to line n of LCD
    }
  }

  timer_clearInterrupt(n);  // clear timer interrupt
}

void timer_start(int n) {  // timer_start(0), 1, etc.
  TIMER* tp = &timer[n];
  kprintf("timer_start %d base=0x%x\n", n, tp->base);
  *(tp->base + TCNTL) |= 0x80;  // set enable bit 7
}

int timer_clearInterrupt(int n) {  // timer_start(0), 1, etc.
  TIMER* tp = &timer[n];
  *(tp->base + TINTCLR) = 0xFFFFFFFF;
}

void timer_stop(int n) {  // stop a timer
  TIMER* tp = &timer[n];
  *(tp->base + TCNTL) &= 0x7F;  // clear enable bit 7
}