/*
 * SpaceJet OS — USART1 driver  (STM32F405)
 * PA9  = USART1_TX  (AF7)
 * PA10 = USART1_RX  (AF7)
 * APB2 = 84 MHz → BRR = 84 000 000 / 115 200 = 729
 */
#include "stm32f405.h"
#include "uart_stm32.h"

void uart_init(void) {
    RCC_AHB1ENR |= RCC_GPIOAEN;
    RCC_APB2ENR |= RCC_USART1EN;

    gpio_set_mode(GPIOA, 9,  GPIO_AF);
    gpio_set_mode(GPIOA, 10, GPIO_AF);
    gpio_set_af  (GPIOA, 9,  AF7_USART);
    gpio_set_af  (GPIOA, 10, AF7_USART);

    /* BRR = APB2 / baud = 84000000 / 115200 ≈ 729 */
    USART1->BRR = 729;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putc(char c) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint32_t)c;
}
void uart_puts(const char *s) {
    while (*s) { if (*s=='\n') uart_putc('\r'); uart_putc(*s++); }
}
int uart_getc(void) {
    while (!(USART1->SR & USART_SR_RXNE));
    return (int)(USART1->DR & 0xFF);
}
int uart_getc_nb(void) {
    if (!(USART1->SR & USART_SR_RXNE)) return -1;
    return (int)(USART1->DR & 0xFF);
}
