#include <stdlib.h>
#include <stdarg.h>

#include "uart_int.h"
#include "defines.h"
#include "exceptions.h"

/************* uart.c file ****************/
UART uart[4];  // 4 UART structures

int uart_init() {
  int i;
  UART* up;
  for (i = 0; i < 3; i++) {  // uart0 to uart2 are adjacent
    up = &uart[i];
    up->base = (char*)(0x101F1000 + i * 0x1000);
    *(up->base + CNTL) &= ~0x10;  // disable UART FIFO
    *(up->base + IMSC) |= 0x30;
    up->n = i;  // UART ID number
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;
    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    up->txon = 0;
  }
  uart[3].base = (char*)(0x10009000);  // uart3 at 0x10009000
  up = & uart[3];
  *(up->base + CNTL) &= ~0x10;  // disable UART FIFO
  *(up->base + IMSC) |= 0x30;
  up->n = i;  // UART ID number
  up->indata = up->inhead = up->intail = 0;
  up->inroom = SBUFSIZE;
  up->outdata = up->outhead = up->outtail = 0;
  up->outroom = SBUFSIZE;
  up->txon = 0;
}

void uart_handler(UART* up) {
  u8 mis = *(up->base + MIS);  // read MIS register
  if (mis & (1 << 4))          // MIS.bit4=RX interrupt
    do_rx(up);
  if (mis & (1 << 5))  // MIS.bit5=TX interrupt
    do_tx(up);
}

int do_rx(UART* up) { // RX interrupt handler
  char c;
  c = *(up->base + UDR);
  printf("RX inttrups: %c\n", c);
  if (c == 0xD) printf("\n");
  up->inbuf[up->inhead++] = c;
  up->inhead %= SBUFSIZE;
  up->indata++;
  up->inroom--;
}

int do_tx(UART* up) { // TX interrupt handler
  char c;
  printf("TX interrupt\n");
  if (up->outdata <= 0) {       // if outbuf[ ] is empty
    *(up->base + IMSC) = 0x10;  // disable TX interrupt
    up->txon = 0;               // turn of txon flag
    return -1;
  }

  c = up->outbuf[up->outtail++];
  up->outtail %= SBUFSIZE;
  *(up->base + UDR) = (int)c;  // write c to DR
  up->outdata--;
  up->outroom++;
  return 0;
}

int ugetc(UART* up) { // return a char from UART
  char c;
  while (up->indata <= 0);  // loop until up->data > 0 READONLY
  c = up->inbuf[up->intail++];
  up->intail %= SBUFSIZE;
  // updating variables: must disable interrupts
  lock();
  up->indata--;
  up->inroom++;
  unlock();
  return c;
}

int uputc(UART* up, char c) { // output a char to UART
  printf("uputc %c ", c);
  if (up->txon) {  // if TX is on, enter c into outbuf[]
    up->outbuf[up->outhead++] = c;
    up->outhead %= 128;
    lock();
    up->outdata++;
    up->outroom--;
    unlock();
    return 0;
  }

  // txon==0 means TX is off => output c & enable TX interrupt
  // PL011 TX is riggered only if write char, else no TX interrupt
  int i = *(up->base + UFR);         // read FR
  while (*(up->base + UFR) & 0x20);  // loop while FR=TXF
  *(up->base + UDR) = (int)c;        // write c to DR
  *(up->base + IMSC) |= 0x30;        // 0000 0000: bit5=TX mask bit4=RX mask
  up->txon = 1;
}

int ugets(UART* up, char* s) { // get a line from UART
  printf("%s", "in ugtes: ");
  while ((*s = (char)ugetc(up)) != '\r') {
    uputc(up, *s++);
  }
  *s = 0;
}

int uprints(UART* up, char* s) { // print a line to UART
  while (*s) uputc(up, *s++);
}

#define MAX_STRING_LEN 255
int uprintu(UART* up, uint32_t val) {
  char output[MAX_STRING_LEN];
  uprints(up, itoa(val, output, 10));
}

int uprintd(UART* up, int val) {
  char output[MAX_STRING_LEN];
  uprints(up, itoa(val, output, 10));
}

int uprintx(UART* up, uint32_t val) {
  char output[MAX_STRING_LEN];
  uprints(up, itoa(val, output, 16));
}

int uprintf(UART* up, const char* fmt, ...) {
  const char* cp = fmt;            // cp points to the fmt string
  int* ip = (int*)&fmt + 1;  // ip points to first item in stack
  while (*cp) {              // scan the format string
    if (*cp != '%') {        // spit out ordinary chars
      uputc(up, *cp);
      if (*cp == '\n')    // for each ‘\n’
        uputc(up, '\r');  // print a ‘\r’
      cp++;
      continue;
    }
    cp++;           // cp points at a conversion symbol
    switch (*cp) {  // print item by %FORMAT symbol
      case 'c':
        uputc(up, (char)*ip);
        break;
      case 's':
        uprints(up, (char*)*ip);
        break;
      case 'u':
        uprintu(up, (uint32_t)*ip);
        break;
      case 'd':
        uprintd(up, (int)*ip);
        break;
      case 'x':
        uprintx(up, (uint32_t)*ip);
        break;
    }
    cp++;
    ip++;  // advance pointers
  }
}