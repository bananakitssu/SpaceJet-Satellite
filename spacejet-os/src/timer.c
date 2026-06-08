/*
 * SpaceJet OS — SP804 Dual Timer Driver  (v2: adds soft preemption)
 * Timer0 base : 0x101E2000  (versatilepb)
 * Input clock : 1 MHz  (QEMU SP804 default)
 */
#include "timer.h"
#include "types.h"

#define TIMER0_BASE   0x101E2000U
#define T_LOAD   MMIO32(TIMER0_BASE + 0x00)
#define T_VALUE  MMIO32(TIMER0_BASE + 0x04)
#define T_CTRL   MMIO32(TIMER0_BASE + 0x08)
#define T_INTCLR MMIO32(TIMER0_BASE + 0x0C)

#define TCTRL_ENABLE    BIT(7)
#define TCTRL_PERIODIC  BIT(6)
#define TCTRL_INTEN     BIT(5)
#define TCTRL_32BIT     BIT(1)

#define TIMER_CLK_HZ  1000000U

volatile uint32_t g_ticks            = 0;
volatile uint8_t  g_preempt_request  = 0;   /* set by ISR, cleared by yield */

void timer_init(uint32_t hz) {
    T_CTRL  = 0;
    T_LOAD  = TIMER_CLK_HZ / hz;
    T_CTRL  = TCTRL_ENABLE | TCTRL_PERIODIC | TCTRL_INTEN | TCTRL_32BIT;
}

void timer_clear_irq(void) {
    T_INTCLR         = 1;
    g_ticks++;
    g_preempt_request = 1;   /* request cooperative preemption */
}

uint32_t timer_ms(void) { return g_ticks * (1000U / TICK_HZ); }
