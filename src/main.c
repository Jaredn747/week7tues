/*************** t.c file *************/
#include <unistd.h>

#include "defines.h"
#include "exceptions.h"
#include "kbd.h"
#include "signal.h"
#include "string.h"
#include "timer.h"
#include "uart_int.h"
#include "vid.h"

extern char _binary____image0_bmp_start;

/*************** C3.3. t.c file *******************/
void copy_vectors(void) {
  extern u32 vectors_start, vectors_end;
  u32* vectors_src = &vectors_start;
  u32* vectors_dst = (u32*)0;
  while (vectors_src < &vectors_end) *vectors_dst++ = *vectors_src++;
}

void IRQ_handler() {  // IRQ interrupt handler in C
  // read VIC status registers to find out which interrupt
  int vicstatus = VIC_STATUS;
  int sicstatus = SIC_STATUS;
  TIMER *tp = timer;
  // VIC status BITs: timer0,1=4,uart0=13,uart1=14
  if (vicstatus & (1 << 5)) {          // bit5:timer2,3
    if (*(tp[2].base + TVALUE) == 0)  // timer 2
      timer_handler(2);
    if (*(tp[3].base + TVALUE) == 0)  // timer 3
      timer_handler(3);
  }
  if (vicstatus & (1 << 4)) {  // bit4=1:timer0,1
    timer_handler(0);
  }
  if (vicstatus & (1 << 12))  // bit12=1: uart0
    uart_handler(&uart[0]);
  if (vicstatus & (1 << 13))  // bit13=1: uart1
    uart_handler(&uart[1]);
  if (vicstatus & (1 << 31)) {   // PIC.bit31= SIC interrupts
    if (sicstatus & (1 << 3)) {  // SIC.bit3 = KBD interrupt
      kbd_handler();
    }
  }
}

int main() {
  char line[128];
  UART* up;
  KBD* kp;

  fbuf_init();  // initialize LCD display
  printf("C3.4 start: test KBD TIMER UART drivers\n");

  char *p = &_binary____image0_bmp_start;
  show_bmp(p, 0, 0);

  /* enable timer0,1, uart0,1 SIC interrupts */
  VIC_INTENABLE = 0;
  VIC_INTENABLE |= (1 << 4);   // timer0,1 at bit4
  VIC_INTENABLE |= (1 << 5);   // timer2,3 at bit5
  VIC_INTENABLE |= (1 << 12);  // UART0 at bit12
  VIC_INTENABLE |= (1 << 13);  // UART1 at bit13
  VIC_INTENABLE |= (1 << 31);  // SIC to VIC's IRQ31
  SIC_INTENABLE = 0;
  SIC_INTENABLE |= (1 << 3);  // KBD int=bit3 on SIC
  kbd_init();                 // initialize keyboard
  uart_init();                // initialize UARTs

  up = &uart[0];              // test UART0 I/O
  kp = &kbd;
  timer_init();
  timer_start(0);  // timer0 only

  while (1) {
    kprintf("Enter a line from KBD\n");
    uprintf(up, "Waiting for KBD...");
    kgets(line);
    uprintf(up, "\nFrom KBD: %s\n", line);

    uprints(up, "Enter a line from UARTS\n\r");
    kprintf("Waiting for UARTS...");
    ugets(up, line);
    uprintf(up, "\n");
    kprintf("\nFrom UARTS: %s\n", line);
  }
}