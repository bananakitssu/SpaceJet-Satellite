#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <stdint.h>
#include "eps.h"
#include "adcs.h"

typedef struct {
    uint32_t uptime_s;
    uint32_t orbit_num;
    int32_t  lat_cdeg;      /* centidegrees ±5160 (±51.60°) */
    int32_t  lon_cdeg;      /* centidegrees ±18000          */
    uint32_t altitude_m;    /* metres ~520 000              */
    uint32_t pkt_count;
} Telemetry;

extern Telemetry g_tlm;
void telemetry_init(void);
void telemetry_task(void);
void telemetry_print(void);
#endif
