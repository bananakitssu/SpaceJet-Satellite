#include "watchdog.h"
#include "scheduler.h"
#include "timer.h"
#include "kprintf.h"
#include "kstring.h"
#include "logger.h"

typedef struct {
    char     name[16];
    uint32_t last_kick;
    uint32_t timeout;
    bool     active;
} WdChannel;

static WdChannel s_wd[WD_MAX_CHANNELS];

int wd_register(const char *name, uint32_t timeout_ticks) {
    for (int i = 0; i < WD_MAX_CHANNELS; i++) {
        if (!s_wd[i].active) {
            kstrncpy(s_wd[i].name, name, 15);
            s_wd[i].last_kick = g_ticks;
            s_wd[i].timeout   = timeout_ticks;
            s_wd[i].active    = true;
            return i;
        }
    }
    return -1;   /* no free channel */
}

void wd_kick(int channel) {
    if (channel >= 0 && channel < WD_MAX_CHANNELS && s_wd[channel].active)
        s_wd[channel].last_kick = g_ticks;
}

void wd_check(void) {
    for (int i = 0; i < WD_MAX_CHANNELS; i++) {
        if (!s_wd[i].active) continue;
        uint32_t age = g_ticks - s_wd[i].last_kick;
        if (age > s_wd[i].timeout) {
            kprintf("\n  [WDT] *** WATCHDOG TIMEOUT: task '%s' (%u ticks silent) ***\n",
                    s_wd[i].name, (unsigned)age);
            logger_write(LOG_ERROR, "Watchdog timeout — controlled reset");
            logger_close_session();
            /* Soft reset: jump to 0x00000000 (vector table → reset handler) */
            kprintf("  [WDT] Resetting...\n");
            void (*rst)(void) = (void(*)(void))0x00000000U;
            rst();
        }
    }
}

void watchdog_task(void) {
    while (1) {
        wd_check();
        task_sleep_ms(1000);   /* check every second */
    }
}
