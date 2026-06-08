/*
 * SpaceJet OS — Electrical Power System (simulated)
 *
 * Simulated orbit: 60 seconds at 100 Hz = 6000 ticks.
 *   Sunlight (0 – 3599): solar panels generating, battery charges.
 *   Eclipse  (3600 – 5999): panels dark, battery discharges.
 */
#include "eps.h"
#include "timer.h"
#include "scheduler.h"
#include "kprintf.h"

EPSState g_eps;

void eps_init(void) {
    g_eps.battery_pct   = 82;
    g_eps.battery_mv    = 3900;
    g_eps.solar_a_mw    = 8500;
    g_eps.solar_b_mw    = 8300;
    g_eps.power_draw_mw = 14200;
    g_eps.temp_batt_c   = 18;
    g_eps.charging      = true;
    g_eps.safe_mode     = false;
}

static void eps_update(void) {
    uint32_t phase = g_ticks % 6000U;   /* one simulated orbit */

    if (phase < 3600U) {
        /* ── sunlight ── */
        g_eps.charging   = true;
        g_eps.solar_a_mw = 8500U + (phase / 40U);
        g_eps.solar_b_mw = 8200U + (phase / 50U);
        if (g_ticks % 20U == 0U && g_eps.battery_pct < 100U)
            g_eps.battery_pct++;
    } else {
        /* ── eclipse ── */
        g_eps.charging   = false;
        g_eps.solar_a_mw = 0;
        g_eps.solar_b_mw = 0;
        if (g_ticks % 40U == 0U && g_eps.battery_pct > 5U)
            g_eps.battery_pct--;
    }
    g_eps.battery_mv    = 3200U + g_eps.battery_pct * 12U;
    g_eps.power_draw_mw = 14000U + (g_ticks % 600U);
    g_eps.temp_batt_c   = 15 + (int32_t)(g_eps.battery_pct / 10U);
}

void eps_print(void) {
    kprintf("  Battery : %u%%  %u mV  [%s]\n",
            g_eps.battery_pct, g_eps.battery_mv,
            g_eps.charging ? "CHARGING" : "DISCHARGING");
    kprintf("  Solar A : %u mW     Solar B : %u mW\n",
            g_eps.solar_a_mw, g_eps.solar_b_mw);
    kprintf("  Draw    : %u mW\n", g_eps.power_draw_mw);
    kprintf("  BattTemp: %d C     Mode: %s\n",
            g_eps.temp_batt_c,
            g_eps.safe_mode ? "SAFE" : "NOMINAL");
}

void eps_task(void) {
    while (1) {
        eps_update();
        task_sleep_ms(500);   /* update twice per second */
    }
}
