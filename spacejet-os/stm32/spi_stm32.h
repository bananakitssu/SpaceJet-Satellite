#ifndef SPI_STM32_H
#define SPI_STM32_H
#include <stdint.h>
/* SPI1 — master mode, CPOL=0, CPHA=0, 8-bit, fPCLK/8 */
void    spi1_init(void);
uint8_t spi1_transfer(uint8_t byte);   /* send + receive simultaneously */
void    spi1_cs_low(void);             /* PA4 chip select */
void    spi1_cs_high(void);
#endif
