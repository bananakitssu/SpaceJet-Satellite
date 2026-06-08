/*
 * SpaceJet OS — STM32F405 Clock Init
 * HSI (16 MHz internal) → PLL → 168 MHz system clock
 * Uses HSI so it works without an external crystal.
 * Swap PLLSRC to HSE (bit 22 of RCC_PLLCFGR) if your board has 8 MHz crystal.
 *
 *  VCO input  = HSI / M     = 16 / 8  = 2 MHz
 *  VCO output = VCO * N     = 2 * 168 = 336 MHz
 *  SYSCLK     = VCO / P     = 336 / 2 = 168 MHz
 *  USB clock  = VCO / Q     = 336 / 7 = 48 MHz
 */
#include "stm32f405.h"
#include "clock.h"

void clock_init(void) {
    /* 1. Enable HSI (should already be on after reset) */
    RCC_CR |= BIT(0);   /* HSION */
    while (!(RCC_CR & BIT(1)));   /* wait HSIRDY */

    /* 2. Set Flash latency for 168 MHz, 3.3V: 5 wait states */
    FLASH_ACR = (FLASH_ACR & ~0xFU) | FLASH_LATENCY_5WS;
    FLASH_ACR |= BIT(8) | BIT(9) | BIT(10);  /* DCEN, ICEN, PRFTEN */

    /* 3. Configure PLL: M=8, N=168, P=2, Q=7, SRC=HSI */
    RCC_PLLCFGR = (8U)          |   /* M */
                  (168U << 6)   |   /* N */
                  (0U  << 16)   |   /* P = /2  (00 = div2) */
                  (0U  << 22)   |   /* SRC = HSI (bit22=0) */
                  (7U  << 24);      /* Q */

    /* 4. Enable PLL */
    RCC_CR |= BIT(24);   /* PLLON */
    while (!(RCC_CR & BIT(25)));   /* wait PLLRDY */

    /* 5. Configure bus prescalers: AHB=/1, APB1=/4, APB2=/2 */
    RCC_CFGR = (0U << 4)  |   /* HPRE  = /1  */
               (5U << 10) |   /* PPRE1 = /4  (101) */
               (4U << 13);    /* PPRE2 = /2  (100) */

    /* 6. Switch SYSCLK to PLL */
    RCC_CFGR |= 2U;   /* SW = PLL */
    while (((RCC_CFGR >> 2) & 3U) != 2U);   /* wait SWS = PLL */
}
