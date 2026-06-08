/*
 * SpaceJet OS — main()  v2.0
 * Boot order:
 *   uart → vic → timer → BIST → sjfs → satkey → logger
 *   → eps/adcs/tlm init → create tasks → enable IRQ → scheduler
 */
#include "types.h"
#include "uart.h"
#include "kprintf.h"
#include "timer.h"
#include "vic.h"
#include "scheduler.h"
#include "eps.h"
#include "adcs.h"
#include "telemetry.h"
#include "command.h"
#include "sjfs.h"
#include "satkey.h"
#include "logger.h"
#include "watchdog.h"
#include "ccsds.h"

static const char *BANNER =
    "\r\n"
    "  ╔══════════════════════════════════════════════╗\r\n"
    "  ║   SpaceJet Satellite OS  v2.0.0              ║\r\n"
    "  ║   ARM926EJ-S  QEMU versatilepb               ║\r\n"
    "  ║   Features: SJFS | SatKey | CCSDS | WDT      ║\r\n"
    "  ╚══════════════════════════════════════════════╝\r\n\r\n";

static void bist(void) {
    kprintf("  [BIST] UART       : OK\n");
    kprintf("  [BIST] Timer IRQ  : ");
    uint32_t t0 = g_ticks, wd = 0;
    while (g_ticks == t0) if (++wd > 10000000U) { kprintf("FAIL\n"); while(1); }
    kprintf("OK (tick %u)\n", (unsigned)g_ticks);
    kprintf("  [BIST] SJFS base  : 0x%08x\n", (unsigned)SJFS_BASE_ADDR);
    kprintf("  [BIST] All checks passed.\n\n");
}

int main(void) {
    uart_init();
    uart_puts(BANNER);

    vic_init();
    timer_init(TICK_HZ);
    vic_enable_irq(VIC_IRQ_TIMER0);
    cpu_irq_enable();

    bist();

    /* Storage and security */
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

    /* Register watchdog channels (tasks kick these in their loops) */
    /* Shell + telemetry tasks are self-monitoring via wd_kick calls */
    kprintf("  [INIT] 5 tasks created.\n\n");

    kprintf("  [BOOT] Scheduler starting...\n\n");
    scheduler_start();
    return 0;
}
