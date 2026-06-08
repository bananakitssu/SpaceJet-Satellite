#ifndef LOGGER_H
#define LOGGER_H
#include <stdint.h>

/*
 * Log path layout:
 *   /_logs/dd_mm_yyyy/hh.mm/logs          ← during session
 *   /_logs/dd_mm_yyyy/hh.mm_HH.MM/logs   ← after shutdown (folder renamed)
 *
 * Date/time is simulated: hardcoded boot date + elapsed seconds from g_ticks.
 * On real hardware, read a DS3231 or equivalent RTC over I2C.
 */

typedef enum {
    LOG_INFO  = 0,
    LOG_WARN  = 1,
    LOG_ERROR = 2,
    LOG_DEBUG = 3,
} LogLevel;

void logger_init(void);
void logger_write(LogLevel lvl, const char *msg);
void logger_writef(LogLevel lvl, const char *fmt, ...);
void logger_close_session(void);   /* called at shutdown — renames folder */
void logger_list_sessions(void);   /* lists sessions via kprintf           */

/* Boot date — adjust to today's date or tie to RTC on real hardware */
#define LOG_BOOT_DAY    7
#define LOG_BOOT_MONTH  6
#define LOG_BOOT_YEAR   2026
#define LOG_BOOT_HOUR   0
#define LOG_BOOT_MIN    0

#endif
