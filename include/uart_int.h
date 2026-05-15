#ifndef __UART_H__
#define __UART_H__

#define UDR 0x00
#define UDS 0x04
#define UFR 0x18
#define CNTL 0x2C
#define IMSC 0x38
#define MIS 0x40
#define SBUFSIZE 128

#include <stdarg.h>

typedef volatile struct uart {
  char* base;  // base address; as char *
  int n;       // uart number 0-3
  char inbuf[SBUFSIZE];
  int indata, inroom, inhead, intail;
  char outbuf[SBUFSIZE];
  int outdata, outroom, outhead, outtail;
  volatile int txon;  // 1=TX interrupt is on
} UART;

extern UART uart[4];

extern void uart_handler(UART* up);
extern int do_rx(UART* up);
extern int do_tx(UART* up);
extern int uart_init();
extern int uprints(UART* up, char* s);
extern int ugets(UART* up, char* s);
extern int uprintf(UART* up, const char* fmt, ...);
extern int vuprintf(UART *up, const char * fmt, va_list args);
#endif