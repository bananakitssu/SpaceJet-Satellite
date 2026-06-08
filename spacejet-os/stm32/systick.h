#ifndef SYSTICK_H
#define SYSTICK_H
#include <stdint.h>
#define TICK_HZ       100U               /* 100 Hz — 10 ms per tick      */
extern volatile uint32_t g_ticks;
void     systick_init(void);
uint32_t timer_ms(void);
void     timer_clear_irq(void);          /* called from SysTick_Handler  */
#endif
