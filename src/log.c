#include <stdarg.h>

#include "log.h"
#include "uart_int.h"

char *levels[] = {
    "ERROR",
    "WARN ",
    "DEBUG",
    "INFO "
};

void log_message(UART *uart, int level, const char *fmt_msg, ...) {
    uprintf(uart, "%s: ", levels[level]);

    va_list args;
    va_start(args, fmt_msg);
    vuprintf(uart, fmt_msg, args);
    va_end(args);
}