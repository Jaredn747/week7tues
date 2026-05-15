#include <stdlib.h>
#include <stdarg.h>

#include "uart_int.h"
#include "defines.h"
#include "exceptions.h"
#include "log_config.h"

/************* uart_int.c file ****************/

UART uart[4];  // 4 UART structures

int uart_init() {
  int i;
  UART* up;

  for (i = 0; i < 3; i++) {  // uart0 to uart2 are adjacent
    up = &uart[i];

    up->base = (char*)(0x101F1000 + i * 0x1000);

    *(up->base + CNTL) &= ~0x10;  // disable UART FIFO
    *(up->base + IMSC) |= 0x30;   // enable RX and TX interrupt masks

    up->n = i;  // UART ID number

    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;

    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;

    up->txon = 0;
  }

  uart[3].base = (char*)(0x10009000);  // uart3 at 0x10009000
  up = &uart[3];

  *(up->base + CNTL) &= ~0x10;  // disable UART FIFO
  *(up->base + IMSC) |= 0x30;   // enable RX and TX interrupt masks

  up->n = 3;  // UART ID number

  up->indata = up->inhead = up->intail = 0;
  up->inroom = SBUFSIZE;

  up->outdata = up->outhead = up->outtail = 0;
  up->outroom = SBUFSIZE;

  up->txon = 0;

  return 0;
}

void uart_handler(UART* up) {
  u8 mis = *(up->base + MIS);  // read MIS register

  if (mis & (1 << 4)) {        // MIS.bit4 = RX interrupt
    do_rx(up);
  }

  if (mis & (1 << 5)) {        // MIS.bit5 = TX interrupt
    do_tx(up);
  }
}

int do_rx(UART* up) {  // RX interrupt handler
  char c;

  c = *(up->base + UDR);

  if (c == 0xD) {
    uputc(up, '\n');
  }

  up->inbuf[up->inhead++] = c;
  up->inhead %= SBUFSIZE;

  up->indata++;
  up->inroom--;

  return 0;
}

int do_tx(UART* up) {  // TX interrupt handler
  char c;

  if (up->outdata <= 0) {       // if outbuf[] is empty
    *(up->base + IMSC) = 0x10;  // disable TX interrupt, keep RX interrupt
    up->txon = 0;               // turn off txon flag
    return -1;
  }

  c = up->outbuf[up->outtail++];
  up->outtail %= SBUFSIZE;

  *(up->base + UDR) = (int)c;   // write c to DR

  /*
   * This is the important homework fix.
   * txon should be turned on here, not in uputc().
   */
  up->txon = 1;

  up->outdata--;
  up->outroom++;

  return 0;
}

int ugetc(UART* up) {  // return a char from UART
  char c;

  while (up->indata <= 0);  // wait until data is available

  c = up->inbuf[up->intail++];
  up->intail %= SBUFSIZE;

  lock();
  up->indata--;
  up->inroom++;
  unlock();

  return c;
}

int uputc(UART* up, char c) {  // output a char to UART
  if (up->txon) {  // if TX is on, enter c into outbuf[]
    up->outbuf[up->outhead++] = c;
    up->outhead %= SBUFSIZE;

    lock();
    up->outdata++;
    up->outroom--;
    unlock();

    return 0;
  }

  /*
   * txon == 0 means TX is off.
   * Write the first character directly to UART,
   * then enable the TX interrupt.
   */
  while (*(up->base + UFR) & 0x20);  // wait while TX FIFO is full

  *(up->base + UDR) = (int)c;        // write c to DR
  *(up->base + IMSC) |= 0x30;        // enable TX and RX interrupt masks

  /*
   * Do NOT put up->txon = 1 here.
   * The homework fix moves that line into do_tx().
   */

  return 0;
}

int ugets(UART* up, char* s) {  // get a line from UART
  while ((*s = (char)ugetc(up)) != '\r') {
    uputc(up, *s++);
  }

  *s = 0;

  return 0;
}

int uprints(UART* up, char* s) {  // print a string to UART
  while (*s) {
    uputc(up, *s++);
  }

  return 0;
}

#define MAX_STRING_LEN 255

int uprintu(UART* up, uint32_t val) {
  char output[MAX_STRING_LEN];

  uprints(up, itoa(val, output, 10));

  return 0;
}

int uprintd(UART* up, int val) {
  char output[MAX_STRING_LEN];

  uprints(up, itoa(val, output, 10));

  return 0;
}

int uprintx(UART* up, uint32_t val) {
  char output[MAX_STRING_LEN];

  uprints(up, itoa(val, output, 16));

  return 0;
}

int uprintf(UART* up, const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vuprintf(up, fmt, args);
  va_end(args);

  return 0;
}

int vuprintf(UART* up, const char* fmt, va_list args) {
  const char* cp = fmt;

  while (*cp) {
    if (*cp != '%') {
      uputc(up, *cp);

      if (*cp == '\n') {
        uputc(up, '\r');
      }

      cp++;
      continue;
    }

    cp++;

    switch (*cp) {
      case 'c':
        uputc(up, va_arg(args, int));
        break;

      case 's':
        uprints(up, va_arg(args, char*));
        break;

      case 'u':
        uprintu(up, va_arg(args, uint32_t));
        break;

      case 'd':
        uprintd(up, va_arg(args, int));
        break;

      case 'x':
        uprintx(up, va_arg(args, uint32_t));
        break;

      default:
        uputc(up, '%');
        uputc(up, *cp);
        break;
    }

    cp++;
  }

  return 0;
}