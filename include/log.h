#ifndef __LOG_H__
#define __LOG_H__
#include <stdarg.h>
#include "uart_int.h"

#ifndef LOG_UART_INDEX
#define LOG_UART_INDEX 0
#endif

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_INFO  4

#ifndef LOG_LEVEL_MAX
#define LOG_LEVEL_MAX 0
#endif

void log_message(UART* uart, int level, const char *fmt, ...);

#if LOG_LEVEL_MAX == 0
#define LOG_ERROR(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL_MAX == 1
#define LOG_ERROR(fmt, ...) log_message(uart[LOG_UART_INDEX], 0, fmt, ##__VA_ARGS__) 
#define LOG_WARN(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL_MAX == 2
#define LOG_ERROR(fmt, ...) log_message(uart[LOG_UART_INDEX], 0, fmt, ##__VA_ARGS__) 
#define LOG_WARN(fmt, ...) log_message(uart[LOG_UART_INDEX], 1, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL_MAX == 3
#define LOG_ERROR(fmt, ...) log_message(uart[LOG_UART_INDEX], 0, fmt, ##__VA_ARGS__) 
#define LOG_WARN(fmt, ...) log_message(uart[LOG_UART_INDEX], 1, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_message(uart[LOG_UART_INDEX], 2, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL_MAX == 4
#define LOG_ERROR(fmt, ...) log_message(&uart[LOG_UART_INDEX], 0, fmt, ##__VA_ARGS__) 
#define LOG_WARN(fmt, ...) log_message(&uart[LOG_UART_INDEX], 1, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_message(&uart[LOG_UART_INDEX], 2, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_message(&uart[LOG_UART_INDEX], 3, fmt, ##__VA_ARGS__)
#endif

#endif