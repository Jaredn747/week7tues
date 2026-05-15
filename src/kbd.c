/******** kbd.c file ******************/
#define USE_KEYSET2
#include "kbd.h"

#include "defines.h"
#include "exceptions.h"
#include "keymap.h"
#include "vid.h"

volatile KBD kbd;
int shifted = 0;
int release = 0;
int control = 0;
volatile int keyset;

int kbd_init() {
  char scode;
  keyset = 2;  // default to scan code set 1
  KBD* kp = &kbd;
  kp->base = (char*)0x10006000;
  *(kp->base + KCNTL) = 0x10;  // bit4=Enable bit0=INT on
  *(kp->base + KCLK) = 8;
  kp->head = kp->tail = 0;
  kp->data = 0;
  kp->room = 128;
  // KBD driver state variables
  shifted = 0;
  release = 0;
  control = 0;
}

// kbd_handelr2() for scan code set 2
void kbd_handler() {
  u8 scode, c;
  KBD* kp = &kbd;
  color = CYAN;
  scode = *(kp->base + KDATA);

  if (scode == 0xF0) {  // key release
    release = 1;
    return;
  }
  if (release && scode != 0x12) {  // ordinay key release
    release = 0;
    return;
  }
  if (release && scode == 0x12) {  // Left shift key release
    release = 0;
    shifted = 0;
    return;
  }
  if (!release && scode == 0x12) {  // left key press and HOLD
    release = 0;
    shifted = 1;
    return;
  }
  if (!release && scode == LCTRL) {  // left Control key press and HOLD
    release = 0;
    control = 1;
    return;
  }
  if (release && scode == LCTRL) {  // Left Control key release
    release = 0;
    control = 0;
    return;
  }
  /********* catch Control-C ****************/
  if (control && scode == 0x21) {  // Control-C
    // send number 2 signal to processes on KBD
    printf("Control-C: %d\n", control);
    control = 0;
    return;
  }
  if (!shifted)
    c = ltab[scode];  // lowercase
  else
    c = utab[scode];  // uppercase
  if (c != '\r') printf("kbd interrupt: c=%c\n", c); 
  if (control && scode == 0x23) {  // Control-D
    c = 0x04;
    printf("Control-D: c = %x\n", c);
  }
  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++;
  kp->room--;
}

int kgetc() {
  char c;
  KBD* kp = &kbd;
  unlock();
  while (kp->data == 0);
  lock();
  c = kp->buf[kp->tail++];
  kp->tail %= 128;
  kp->data--;
  kp->room++;
  unlock();
  return c;
}

int kgets(char s[]) {
  char c;
  while ((c = kgetc()) != '\r') {
    if (c == '\b') {
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}