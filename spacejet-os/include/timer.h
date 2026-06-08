#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
extern volatile uint32_t g_ticks;   /* incremented by timer ISR at TICK_HZ */
#define TICK_HZ 100                 /* 100 Hz → 10 ms per tick             */
void     timer_init(uint32_t hz);
void     timer_clear_irq(void);
uint32_t timer_ms(void);            /* elapsed milliseconds since boot      */
#endif
extern volatile uint8_t g_preempt_request;
