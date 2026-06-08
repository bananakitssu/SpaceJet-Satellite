#ifndef ADCS_H
#define ADCS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int32_t  roll_mdeg;    /* millidegrees  ±500  */
    int32_t  pitch_mdeg;
    int32_t  yaw_mdeg;
    int32_t  rw_x_rpm;    /* reaction wheel RPM  */
    int32_t  rw_y_rpm;
    int32_t  rw_z_rpm;
    bool     locked;       /* attitude solution locked? */
    uint8_t  mode;         /* 0=DETUMBLE 1=NADIR 2=SUN  */
} ADCSState;

extern ADCSState g_adcs;
void adcs_init(void);
void adcs_task(void);
void adcs_print(void);
#endif
