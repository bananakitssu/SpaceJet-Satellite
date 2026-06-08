/*
 * SpaceJet OS — Telemetry Task
 * Aggregates EPS + ADCS + simulated orbital state.
 * Prints a status beacon every 5 s (500 ticks at 100 Hz).
 */
#include "telemetry.h"
#include "timer.h"
#include "scheduler.h"
#include "kprintf.h"

Telemetry g_tlm;

void telemetry_init(void) {
    g_tlm.uptime_s   = 0;
    g_tlm.orbit_num  = 1;
    g_tlm.lat_cdeg   = 0;
    g_tlm.lon_cdeg   = 0;
    g_tlm.altitude_m = 520000;
    g_tlm.pkt_count  = 0;
}

static void tlm_update(void) {
    g_tlm.uptime_s  = g_ticks / TICK_HZ;
    g_tlm.orbit_num = 1 + g_ticks / 6000;  /* 60 s per simulated orbit */

    /* Latitude: triangle wave ±5160 cdeg over orbit period */
    uint32_t phase = g_ticks % 6000U;
    int32_t lat;
    if      (phase < 1500) lat =  (int32_t)(phase * 5160U / 1500U);
    else if (phase < 3000) lat =  (int32_t)((3000U - phase) * 5160U / 1500U);
    else if (phase < 4500) lat = -(int32_t)((phase - 3000U) * 5160U / 1500U);
    else                   lat = -(int32_t)((6000U - phase) * 5160U / 1500U);
    g_tlm.lat_cdeg = lat;

    /* Longitude: drifts steadily */
    g_tlm.lon_cdeg   = (int32_t)((g_ticks * 3U) % 36000U) - 18000;
    g_tlm.altitude_m = 520000 + (g_ticks % 2000);
    g_tlm.pkt_count++;
}

void telemetry_print(void) {
    uint32_t s   = g_tlm.uptime_s;
    uint32_t h   = s / 3600;
    uint32_t m   = (s % 3600) / 60;
    uint32_t sec = s % 60;

    int32_t lat_d  = g_tlm.lat_cdeg / 100;
    int32_t lat_f  = (g_tlm.lat_cdeg < 0 ? -g_tlm.lat_cdeg : g_tlm.lat_cdeg) % 100;
    int32_t lon_d  = g_tlm.lon_cdeg / 100;
    int32_t lon_f  = (g_tlm.lon_cdeg < 0 ? -g_tlm.lon_cdeg : g_tlm.lon_cdeg) % 100;

    kprintf("=== SpaceJet Telemetry [pkt #%u] ===\n", g_tlm.pkt_count);
    kprintf("  Uptime : %03u:%02u:%02u     Orbit : %u\n", h, m, sec, g_tlm.orbit_num);
    kprintf("  Pos    : Lat %d.%02d deg   Lon %d.%02d deg\n",
            lat_d, lat_f, lon_d, lon_f);
    kprintf("  Alt    : %u m\n", g_tlm.altitude_m);
    kprintf("--- EPS ---\n");
    eps_print();
    kprintf("--- ADCS ---\n");
    adcs_print();
    kprintf("====================================\n");
}

void telemetry_task(void) {
    while (1) {
        tlm_update();
        telemetry_print();
        task_sleep_ms(5000);   /* beacon every 5 seconds */
    }
}
