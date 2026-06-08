/*
 * SpaceJet OS — UART Command Shell  (v2: auth, downlink, logs, fs)
 */
#include "command.h"
#include "uart.h"
#include "kprintf.h"
#include "kstring.h"
#include "scheduler.h"
#include "timer.h"
#include "eps.h"
#include "adcs.h"
#include "telemetry.h"
#include "satkey.h"
#include "logger.h"
#include "sjfs.h"
#include "ccsds.h"
#include "watchdog.h"
#include <stdint.h>

/* ── command helpers ── */
static void cmd_help(void) {
    kprintf("\n  SpaceJet OS v2.0 — Commands\n");
    kprintf("  %-14s %s\n","help",     "Show this list");
    kprintf("  %-14s %s\n","status",   "System overview");
    kprintf("  %-14s %s\n","tlm",      "Full telemetry report");
    kprintf("  %-14s %s\n","power",    "EPS subsystem");
    kprintf("  %-14s %s\n","attitude", "ADCS subsystem");
    kprintf("  %-14s %s\n","orbit",    "Orbital position");
    kprintf("  %-14s %s\n","tasks",    "Task scheduler list");
    kprintf("  %-14s %s\n","downlink", "Send CCSDS telemetry packet");
    kprintf("  %-14s %s\n","ticks",    "Raw tick counter");
    kprintf("  %-14s %s\n","fs",       "List SJFS filesystem");
    kprintf("  %-14s %s\n","logs",     "List log sessions");
    kprintf("  %-14s %s\n","version",  "Firmware version");
    kprintf("  --- Privileged (requires auth) ---\n");
    kprintf("  %-14s %s\n","auth <key>","Authenticate with SatKey");
    kprintf("  %-14s %s\n","logout",   "End authenticated session");
    kprintf("  %-14s %s\n","shutdown", "Safe shutdown + log close");
    kprintf("  %-14s %s\n","safemode", "Enable EPS safe mode");
    kprintf("  %-14s %s\n","reboot",   "Soft reset satellite");
    kprintf("  %-14s %s\n","keyinfo",  "Show SatKey storage path");
    kprintf("\n");
}

static void cmd_status(void) {
    uint32_t s = g_ticks/TICK_HZ, h=s/3600, m=(s%3600)/60, sec=s%60;
    kprintf("\n  === SpaceJet Status ===\n");
    kprintf("  Uptime  : %03u:%02u:%02u\n", h, m, sec);
    kprintf("  Battery : %u%% [%s]\n", g_eps.battery_pct,
            g_eps.charging?"CHARGING":"DISCHARGING");
    kprintf("  ADCS    : %s  Roll:%d Pitch:%d Yaw:%d mdeg\n",
            g_adcs.locked?"LOCKED":"SEARCHING",
            g_adcs.roll_mdeg, g_adcs.pitch_mdeg, g_adcs.yaw_mdeg);
    kprintf("  Orbit # : %u   Alt: %u m\n", g_tlm.orbit_num, g_tlm.altitude_m);
    kprintf("  Auth    : %s\n", g_authenticated?"YES (privileged)":"NO");
    kprintf("  Mode    : %s\n", g_eps.safe_mode?"SAFE":"NOMINAL");
    kprintf("  =======================\n\n");
}

static void cmd_orbit(void) {
    int32_t ld=g_tlm.lat_cdeg/100, lf=(g_tlm.lat_cdeg<0?-g_tlm.lat_cdeg:g_tlm.lat_cdeg)%100;
    int32_t nd=g_tlm.lon_cdeg/100, nf=(g_tlm.lon_cdeg<0?-g_tlm.lon_cdeg:g_tlm.lon_cdeg)%100;
    kprintf("\n  Orbit  : %u\n",       g_tlm.orbit_num);
    kprintf("  Lat    : %d.%02d deg\n", ld, lf);
    kprintf("  Lon    : %d.%02d deg\n", nd, nf);
    kprintf("  Alt    : %u m\n\n",     g_tlm.altitude_m);
}

static void cmd_downlink(void) {
    kprintf("  [CCSDS] Building telemetry packet...\n");
    /* Build a simple ASCII payload from telemetry */
    char payload[128];
    ksnprintf(payload, sizeof(payload),
              "BAT=%u%% ALT=%um LAT=%d LON=%d ROLL=%d PITCH=%d YAW=%d",
              (unsigned)g_eps.battery_pct,
              (unsigned)g_tlm.altitude_m,
              (int)g_tlm.lat_cdeg, (int)g_tlm.lon_cdeg,
              (int)g_adcs.roll_mdeg, (int)g_adcs.pitch_mdeg,
              (int)g_adcs.yaw_mdeg);
    CcsdsPacket pkt;
    ccsds_build(&pkt, CCSDS_APID_TLM, 0x01,
                (uint8_t *)payload, (uint16_t)kstrlen(payload));
    kprintf("  [CCSDS] Packet (APID=0x001, seq=%u, %u bytes):\n",
            (unsigned)(pkt.ph_seq & 0x3FFF),
            (unsigned)ccsds_packet_len(&pkt));
    ccsds_send(&pkt);
    logger_write(LOG_INFO, "CCSDS telemetry packet downlinked");
}

static void cmd_version(void) {
    kprintf("\n  SpaceJet Satellite OS v2.0.0\n");
    kprintf("  Target : ARM926EJ-S / QEMU versatilepb\n");
    kprintf("  FS     : SJFS @ 0x%08x\n", (unsigned)SJFS_BASE_ADDR);
    kprintf("  Ticks  : %u (%u Hz)\n\n", (unsigned)g_ticks, (unsigned)TICK_HZ);
}

/* ── Privileged command guard ── */
static bool check_auth(void) {
    if (!g_authenticated) {
        kprintf("  [AUTH] Not authenticated. Run: auth <satkey>\n");
        return false;
    }
    return true;
}

void command_execute(const char *line) {
    /* ── Public commands ── */
    if      (kstrcmp(line,"help")     ==0) cmd_help();
    else if (kstrcmp(line,"status")   ==0) cmd_status();
    else if (kstrcmp(line,"tlm")      ==0) telemetry_print();
    else if (kstrcmp(line,"power")    ==0) { kprintf("\n"); eps_print();  kprintf("\n"); }
    else if (kstrcmp(line,"attitude") ==0) { kprintf("\n"); adcs_print(); kprintf("\n"); }
    else if (kstrcmp(line,"orbit")    ==0) cmd_orbit();
    else if (kstrcmp(line,"tasks")    ==0) { kprintf("\n"); scheduler_print_tasks(); kprintf("\n"); }
    else if (kstrcmp(line,"ticks")    ==0) kprintf("  g_ticks = %u\n\n", (unsigned)g_ticks);
    else if (kstrcmp(line,"downlink") ==0) cmd_downlink();
    else if (kstrcmp(line,"fs")       ==0) { kprintf("\n"); sjfs_list(); kprintf("\n"); }
    else if (kstrcmp(line,"logs")     ==0) { kprintf("\n"); logger_list_sessions(); kprintf("\n"); }
    else if (kstrcmp(line,"version")  ==0) cmd_version();

    /* ── Auth command: auth <hexkey> ── */
    else if (kstrncmp(line,"auth ",5)==0) {
        const char *key = line + 5;
        if (satkey_verify(key)) {
            g_authenticated = true;
            kprintf("  [AUTH] *** Authenticated — welcome, Mission Control. ***\n\n");
            logger_write(LOG_INFO, "Operator authenticated successfully");
        } else {
            kprintf("  [AUTH] Invalid SatKey.\n\n");
            logger_write(LOG_WARN, "Failed authentication attempt");
        }
    }

    /* ── Privileged commands ── */
    else if (kstrcmp(line,"logout")==0) {
        if (g_authenticated) {
            g_authenticated = false;
            kprintf("  [AUTH] Session ended.\n\n");
            logger_write(LOG_INFO, "Operator logged out");
        } else kprintf("  Not authenticated.\n");
    }
    else if (kstrcmp(line,"keyinfo")==0) {
        if (!check_auth()) return;
        kprintf("\n"); satkey_print_path(); kprintf("\n");
    }
    else if (kstrcmp(line,"safemode")==0) {
        if (!check_auth()) return;
        g_eps.safe_mode = !g_eps.safe_mode;
        kprintf("  [EPS] Safe mode: %s\n\n", g_eps.safe_mode?"ON":"OFF");
        logger_write(LOG_WARN, g_eps.safe_mode ? "Safe mode ENABLED" : "Safe mode DISABLED");
    }
    else if (kstrcmp(line,"shutdown")==0) {
        if (!check_auth()) return;
        kprintf("  [SYS] Authenticated shutdown initiated...\n");
        logger_write(LOG_INFO, "Operator-initiated shutdown");
        logger_close_session();
        kprintf("  [SYS] Logs saved. Halting.\n");
        while(1);   /* halt — on real hardware this would cut power */
    }
    else if (kstrcmp(line,"reboot")==0) {
        if (!check_auth()) return;
        kprintf("  [SYS] Rebooting...\n");
        logger_write(LOG_INFO, "Operator-initiated reboot");
        logger_close_session();
        void (*rst)(void) = (void(*)(void))0x00000000U;
        rst();
    }
    else kprintf("  Unknown: '%s'  (try 'help')\n\n", line);
}

/* ── Shell task ── */
void command_task(void) {
    static char buf[72];
    static int  pos = 0;

    kprintf("  Type 'help' for commands\n");
    kprintf("sj> ");

    while (1) {
        int c = uart_getc_nb();
        if (c < 0) { preempt_check(); scheduler_yield(); continue; }

        if (c == '\r' || c == '\n') {
            kprintf("\n");
            buf[pos] = '\0';
            if (pos > 0) {
                command_execute(buf);
                logger_writef(LOG_DEBUG, "CMD: %s", buf);
            }
            pos = 0;
            kprintf("sj> ");
        } else if ((c == 127 || c == '\b') && pos > 0) {
            pos--; uart_puts("\b \b");
        } else if (c >= 32 && c < 127 && pos < 71) {
            buf[pos++] = (char)c; uart_putc((char)c);
        }
    }
}
