/*
 * SpaceJet OS — SysTick (STM32F405)
 * Fires every 10 ms (TICK_HZ = 100).
 * Each tick also sets PENDSVSET to trigger the preemptive context switch.
 */
#include "stm32f405.h"
#include "systick.h"

volatile uint32_t g_ticks = 0;

void systick_init(void) {
    /* Reload for TICK_HZ at SYS_CLOCK_HZ */
    SYSTICK->LOAD = (168000000U / TICK_HZ) - 1U;
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE
                  | SYSTICK_CTRL_TICKINT
                  | SYSTICK_CTRL_CLKSRC;   /* processor clock */

    /* PendSV = lowest priority (0xFF), SysTick = 0xF0 */
    SCB_SHPR3 = (0xF0U << 24) | (0xFFU << 16);
}

/* Called by SysTick_Handler (defined in startup_m4.s as weak, overridden here) */
void timer_clear_irq(void) {
    g_ticks++;
    SCB_ICSR |= ICSR_PENDSVSET;   /* ← trigger PendSV = preemptive switch! */
}

uint32_t timer_ms(void) { return g_ticks * (1000U / TICK_HZ); }
