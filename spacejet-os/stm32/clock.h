#ifndef CLOCK_H
#define CLOCK_H
#include <stdint.h>
#define SYS_CLOCK_HZ   168000000U  /* 168 MHz — STM32F405 max   */
#define APB1_CLOCK_HZ   42000000U  /* APB1 = SYS/4              */
#define APB2_CLOCK_HZ   84000000U  /* APB2 = SYS/2              */
void clock_init(void);             /* HSE 8MHz → PLL → 168 MHz  */
#endif
