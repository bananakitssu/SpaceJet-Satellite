/*
 * SpaceJet OS — Minimal kernel printf
 * Uses compiler-provided <stdarg.h> — no libc needed.
 * Division/modulo calls __aeabi_uidiv from -lgcc.
 */
#include "kprintf.h"
#include "uart.h"
#include <stdarg.h>
#include <stdint.h>

static void put_uint(uint32_t n, uint32_t base, int width, char pad) {
    char buf[32];
    int  i = 0;
    const char *digits = "0123456789abcdef";
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n) { buf[i++] = digits[n % base]; n /= base; }
    }
    while (i < width) buf[i++] = pad;   /* left-fill (will be reversed) */
    while (i--) uart_putc(buf[i]);       /* print high digit first       */
}

static void put_int(int32_t n, int width, char pad) {
    if (n < 0) { uart_putc('-'); put_uint((uint32_t)(-(n+1))+1, 10, width-1, pad); }
    else        { put_uint((uint32_t)n, 10, width, pad); }
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        if (*fmt != '%') {
            if (*fmt == '\n') uart_putc('\r');
            uart_putc(*fmt++);
            continue;
        }
        fmt++;                           /* skip '%' */
        char    pad   = ' ';
        int     width = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') { width = width*10 + (*fmt-'0'); fmt++; }
        switch (*fmt) {
        case 'd': put_int(va_arg(ap, int32_t), width, pad);          break;
        case 'u': put_uint(va_arg(ap, uint32_t), 10, width, pad);    break;
        case 'x': put_uint(va_arg(ap, uint32_t), 16, width, pad);    break;
        case 'p': uart_puts("0x");
                  put_uint(va_arg(ap, uint32_t), 16, 8, '0');        break;
        case 's': { const char *s = va_arg(ap, const char *);
                    if (!s) s = "(null)";
                    while (*s) uart_putc(*s++);                       break; }
        case 'c': uart_putc((char)va_arg(ap, int));                   break;
        case '%': uart_putc('%');                                      break;
        default:  uart_putc('%'); uart_putc(*fmt);                    break;
        }
        fmt++;
    }
    va_end(ap);
}

__attribute__((weak)) int kstrcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

__attribute__((weak)) int kstrlen(const char *s) {
    int n = 0; while (*s++) n++; return n;
}
