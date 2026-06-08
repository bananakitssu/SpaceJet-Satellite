#include "logger.h"
#include "sjfs.h"
#include "timer.h"
#include "kstring.h"
#include "kprintf.h"
#include <stdarg.h>
#include <stdint.h>

/* ── Current session state ── */
static int   s_log_idx    = -1;   /* SJFS file index                */
static char  s_sess_path[SJFS_PATH_LEN];  /* current session folder  */
static bool  s_open       = false;
static uint32_t s_boot_h  = LOG_BOOT_HOUR;
static uint32_t s_boot_m  = LOG_BOOT_MIN;

/* Seconds elapsed since boot */
static uint32_t elapsed_s(void) { return g_ticks / TICK_HZ; }

static void current_hm(uint32_t *h, uint32_t *m) {
    uint32_t total_m = elapsed_s() / 60 + s_boot_h * 60 + s_boot_m;
    *h = (total_m / 60) % 24;
    *m =  total_m % 60;
}

static const char *level_tag(LogLevel l) {
    switch (l) {
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        case LOG_DEBUG: return "DEBUG";
        default:        return "?    ";
    }
}

void logger_init(void) {
    /* Build session start path:  /_logs/dd_mm_yyyy/hh.mm */
    char session_dir[SJFS_PATH_LEN];
    ksnprintf(session_dir, sizeof(session_dir),
              "/_logs/%02u_%02u_%04u/%02u.%02u",
              LOG_BOOT_DAY, LOG_BOOT_MONTH, LOG_BOOT_YEAR,
              LOG_BOOT_HOUR, LOG_BOOT_MIN);

    /* Full path to the log file inside that folder */
    char log_path[SJFS_PATH_LEN];
    ksnprintf(log_path, sizeof(log_path), "%s/logs", session_dir);

    kstrncpy(s_sess_path, session_dir, SJFS_PATH_LEN - 1);

    s_log_idx = sjfs_open_or_create(log_path);
    s_open    = (s_log_idx >= 0);

    if (!s_open) {
        kprintf("  [LOG] ERROR: could not open log file (SJFS full?)\n");
        return;
    }
    kprintf("  [LOG] Session started → %s/logs\n", session_dir);
    logger_write(LOG_INFO, "SpaceJet OS boot — session started");
}

void logger_write(LogLevel lvl, const char *msg) {
    if (!s_open || s_log_idx < 0) return;

    uint32_t h, m;
    current_hm(&h, &m);
    uint32_t s = elapsed_s() % 60;

    char line[160];
    ksnprintf(line, sizeof(line), "[%02u:%02u:%02u] [%s] %s\n",
              h, m, s, level_tag(lvl), msg);
    sjfs_append_str(s_log_idx, line);
}

void logger_writef(LogLevel lvl, const char *fmt, ...) {
    char buf[128];
    va_list ap; va_start(ap, fmt);
    /* Use ksnprintf with va_list — reuse kvprintf approach via kprintf */
    /* For simplicity, write format string only (real impl would expand) */
    ksnprintf(buf, sizeof(buf), "%s", fmt);   /* basic fallback */
    va_end(ap);
    logger_write(lvl, buf);
}

void logger_close_session(void) {
    if (!s_open) return;
    logger_write(LOG_INFO, "SpaceJet OS shutdown — session closing");

    /* Rename the session folder: hh.mm → hh.mm_HH.MM */
    uint32_t h, m;
    current_hm(&h, &m);

    /* Build old log file path and new log file path */
    char old_log[SJFS_PATH_LEN], new_log[SJFS_PATH_LEN];
    char new_dir[SJFS_PATH_LEN];

    ksnprintf(old_log, sizeof(old_log), "%s/logs", s_sess_path);

    ksnprintf(new_dir, sizeof(new_dir),
              "/_logs/%02u_%02u_%04u/%02u.%02u_%02u.%02u",
              LOG_BOOT_DAY, LOG_BOOT_MONTH, LOG_BOOT_YEAR,
              LOG_BOOT_HOUR, LOG_BOOT_MIN,
              (unsigned)h, (unsigned)m);

    ksnprintf(new_log, sizeof(new_log), "%s/logs", new_dir);

    sjfs_rename(old_log, new_log);
    kstrncpy(s_sess_path, new_dir, SJFS_PATH_LEN - 1);
    kprintf("  [LOG] Session closed → %s/logs\n", new_dir);
    s_open = false;
}

void logger_list_sessions(void) {
    kprintf("  Current log session: %s/logs\n", s_sess_path);
    sjfs_list();
}
