/*
 * SpaceJet OS — ADCS Simulator
 * Generates realistic small attitude perturbations.
 * Triangle-wave oscillations are used instead of trig
 * to avoid needing libm.
 */
#include "adcs.h"
#include "timer.h"
#include "scheduler.h"
#include "kprintf.h"

ADCSState g_adcs;

/* Integer triangle wave: period P, amplitude A, returns -A..+A */
static int32_t tri(uint32_t t, uint32_t period, int32_t amp) {
    uint32_t p = t % period;
    int32_t  v = (int32_t)(p < period/2 ? p : period - p);
    return (v * amp * 2) / (int32_t)(period / 2) - amp;
}

void adcs_init(void) {
    g_adcs.roll_mdeg  = 0;
    g_adcs.pitch_mdeg = 0;
    g_adcs.yaw_mdeg   = 0;
    g_adcs.rw_x_rpm   = 1000;
    g_adcs.rw_y_rpm   = 950;
    g_adcs.rw_z_rpm   = 1050;
    g_adcs.locked     = true;
    g_adcs.mode       = 1;   /* NADIR */
}

static void adcs_update(void) {
    uint32_t t = g_ticks;
    /* Slow attitude perturbations (orbital disturbances) */
    g_adcs.roll_mdeg  = tri(t,  800, 450);
    g_adcs.pitch_mdeg = tri(t, 1300, 380);
    g_adcs.yaw_mdeg   = tri(t,  600, 200);
    /* Reaction wheel speed modulation */
    g_adcs.rw_x_rpm   = 1000 + tri(t, 500, 150);
    g_adcs.rw_y_rpm   =  950 + tri(t, 700, 120);
    g_adcs.rw_z_rpm   = 1050 + tri(t, 400, 180);
    /* Lock maintained if attitude error < 600 mdeg */
    int32_t err = g_adcs.roll_mdeg < 0 ? -g_adcs.roll_mdeg : g_adcs.roll_mdeg;
    g_adcs.locked = (err < 600);
}

static const char *mode_name(uint8_t m) {
    if (m == 0) return "DETUMBLE";
    if (m == 1) return "NADIR   ";
    return "SUN-POINT";
}

void adcs_print(void) {
    kprintf("  Roll   : %d mdeg\n",  g_adcs.roll_mdeg);
    kprintf("  Pitch  : %d mdeg\n",  g_adcs.pitch_mdeg);
    kprintf("  Yaw    : %d mdeg\n",  g_adcs.yaw_mdeg);
    kprintf("  RW X/Y/Z: %d / %d / %d RPM\n",
            g_adcs.rw_x_rpm, g_adcs.rw_y_rpm, g_adcs.rw_z_rpm);
    kprintf("  Mode   : %s     Locked: %s\n",
            mode_name(g_adcs.mode), g_adcs.locked ? "YES" : "NO");
}

void adcs_task(void) {
    while (1) {
        adcs_update();
        task_sleep_ms(100);   /* 10 Hz attitude update */
    }
}
