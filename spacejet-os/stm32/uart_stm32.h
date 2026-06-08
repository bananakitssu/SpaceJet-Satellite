#ifndef UART_STM32_H
#define UART_STM32_H
#include <stdint.h>
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
int  uart_getc(void);
int  uart_getc_nb(void);
#endif
