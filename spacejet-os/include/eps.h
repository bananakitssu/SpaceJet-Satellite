#ifndef EPS_H
#define EPS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t battery_pct;     /* 0 – 100 %                  */
    uint32_t battery_mv;      /* mV  (3200 – 4200)          */
    uint32_t solar_a_mw;      /* Solar panel A power (mW)   */
    uint32_t solar_b_mw;      /* Solar panel B power (mW)   */
    uint32_t power_draw_mw;   /* Total consumption (mW)     */
    int32_t  temp_batt_c;     /* Battery temperature (°C)   */
    bool     charging;
    bool     safe_mode;
} EPSState;

extern EPSState g_eps;
void eps_init(void);
void eps_task(void);   /* scheduler task entry */
void eps_print(void);
#endif
