#ifndef WATCHDOG_H
#define WATCHDOG_H
#include <stdint.h>
#include <stdbool.h>

#define WD_MAX_CHANNELS  8
#define WD_DEFAULT_TIMEOUT_TICKS  500   /* 5 s at 100 Hz */

/* Each task registers a watchdog channel and must call wd_kick()
 * periodically.  If any channel goes silent for timeout_ticks,
 * wd_check() triggers a controlled reset. */

int  wd_register(const char *name, uint32_t timeout_ticks);
void wd_kick(int channel);          /* call from task's main loop */
void wd_check(void);                /* call from watchdog_task()  */
void watchdog_task(void);           /* scheduler task entry       */

#endif
