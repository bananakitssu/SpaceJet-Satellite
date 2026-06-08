#include "stm32f405.h"
#include "spi_stm32.h"

/* PA4=CS(GPIO_OUT), PA5=SCK, PA6=MISO, PA7=MOSI (all AF5) */
void spi1_init(void) {
    RCC_AHB1ENR |= RCC_GPIOAEN;
    RCC_APB2ENR |= RCC_SPI1EN;

    /* CS = PA4 as output, default high */
    gpio_set_mode(GPIOA, 4, GPIO_OUT);
    gpio_set(GPIOA, 4);

    /* SCK, MISO, MOSI = AF5 */
    gpio_set_mode(GPIOA, 5, GPIO_AF);
    gpio_set_mode(GPIOA, 6, GPIO_AF);
    gpio_set_mode(GPIOA, 7, GPIO_AF);
    gpio_set_af(GPIOA, 5, AF5_SPI1);
    gpio_set_af(GPIOA, 6, AF5_SPI1);
    gpio_set_af(GPIOA, 7, AF5_SPI1);

    /*
     * CR1: MSTR=1, SSI=1, SSM=1 (software CS), BR=010 (fPCLK/8),
     *      CPOL=0, CPHA=0, 8-bit (DFF=0)
     */
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM |
                (2U << 3);   /* BR = /8 → 84/8 = 10.5 MHz */
    SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t spi1_transfer(uint8_t byte) {
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = byte;
    while (!(SPI1->SR & SPI_SR_RXNE));
    return (uint8_t)(SPI1->DR & 0xFF);
}

void spi1_cs_low(void)  { gpio_clear(GPIOA, 4); }
void spi1_cs_high(void) { gpio_set  (GPIOA, 4); }
