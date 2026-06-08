/*
 * SpaceJet OS — PL011 UART Driver
 * Base address : 0x101F1000  (UART0, versatilepb)
 * Input clock  : 24 MHz  (QEMU default)
 * Baud rate    : 115200
 */
#include "uart.h"
#include "types.h"

#define UART0         0x101F1000U

#define UARTDR        MMIO32(UART0 + 0x000)  /* Data register          */
#define UARTFR        MMIO32(UART0 + 0x018)  /* Flag register          */
#define UARTIBRD      MMIO32(UART0 + 0x024)  /* Integer baud divisor   */
#define UARTFBRD      MMIO32(UART0 + 0x028)  /* Fractional baud divisor*/
#define UARTLCRH      MMIO32(UART0 + 0x02C)  /* Line control           */
#define UARTCR        MMIO32(UART0 + 0x030)  /* Control register       */
#define UARTIMSC      MMIO32(UART0 + 0x038)  /* Interrupt mask         */

/* FR bits */
#define FR_TXFF  BIT(5)   /* TX FIFO full  */
#define FR_RXFE  BIT(4)   /* RX FIFO empty */

/* LCRH: 8-bit word, FIFO enable */
#define LCRH_WLEN8  (3U << 5)
#define LCRH_FEN    BIT(4)

/* CR: UART + TX + RX enable */
#define CR_UARTEN  BIT(0)
#define CR_TXE     BIT(8)
#define CR_RXE     BIT(9)

void uart_init(void) {
    UARTCR   = 0;                          /* disable while configuring     */
    /* 24 MHz / (16 × 115200) = 13.020…   IBRD=13, FBRD=round(0.02×64)=1  */
    UARTIBRD = 13;
    UARTFBRD = 1;
    UARTLCRH = LCRH_WLEN8 | LCRH_FEN;     /* 8N1, FIFO on                 */
    UARTIMSC = 0;                          /* all interrupts masked (poll)  */
    UARTCR   = CR_UARTEN | CR_TXE | CR_RXE;
}

void uart_putc(char c) {
    while (UARTFR & FR_TXFF);   /* spin while TX FIFO full */
    UARTDR = (uint32_t)c;
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

int uart_getc(void) {
    while (UARTFR & FR_RXFE);   /* block until char ready */
    return (int)(UARTDR & 0xFFU);
}

int uart_getc_nb(void) {
    if (UARTFR & FR_RXFE) return -1;
    return (int)(UARTDR & 0xFFU);
}
