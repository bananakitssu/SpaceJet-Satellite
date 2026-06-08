/*
 * SpaceJet OS — STM32F405 main()
 *
 * Boot: clock → uart → systick → sjfs → satkey → logger
 *       → satellite tasks → scheduler_start()
 *
 * True preemptive scheduling: SysTick (10 ms) → PendSV → task switch.
 * No cooperative yield required — tasks can block on UART and still
 * be preempted by the timer.
 */
#include "stm32f405.h"
#include "clock.h"
#include "uart_stm32.h"
#include "systick.h"
#include "scheduler_m4.h"
/* Shared satellite modules (from parent src/) */
#include "../include/kprintf.h"
#include "../include/sjfs.h"
#include "../include/satkey.h"
#include "../include/logger.h"
#include "../include/eps.h"
#include "../include/adcs.h"
#include "../include/telemetry.h"
#include "../include/command.h"
#include "../include/watchdog.h"

/* kprintf needs a putc — point it at USART1 */
/* (kprintf.c calls uart_putc, which resolves to uart_stm32.c on STM32) */

static const char *BANNER =
    "\r\n"
    "  ╔══════════════════════════════════════════════╗\r\n"
    "  ║   SpaceJet Satellite OS  v2.0.0              ║\r\n"
    "  ║   STM32F405RG  @168 MHz  Cortex-M4           ║\r\n"
    "  ║   TRUE Preemptive: SysTick → PendSV          ║\r\n"
    "  ╚══════════════════════════════════════════════╝\r\n\r\n";

int main(void) {
    clock_init();      /* HSI → PLL → 168 MHz                         */
    uart_init();       /* USART1 PA9/PA10, 115200                     */
    uart_puts(BANNER);

    kprintf("  [INIT] Clock   : 168 MHz\n");
    kprintf("  [INIT] USART1  : 115200 baud\n");

    systick_init();    /* 100 Hz tick, PendSV priority set            */
    kprintf("  [INIT] SysTick : 100 Hz (PendSV preemption active)\n");

    /* Storage + security */
    sjfs_init();
    satkey_init();
    logger_init();

    /* Satellite subsystems */
    eps_init();
    adcs_init();
    telemetry_init();

    kprintf("  [INIT] Creating tasks...\n");
    task_create("shell",     command_task);
    task_create("telemetry", telemetry_task);
    task_create("eps",       eps_task);
    task_create("adcs",      adcs_task);
    task_create("watchdog",  watchdog_task);
    kprintf("  [INIT] 5 tasks — TRUE PREEMPTIVE via PendSV\n\n");

    kprintf("  [BOOT] Scheduler starting...\n\n");
    scheduler_start();   /* never returns */
    return 0;
}
