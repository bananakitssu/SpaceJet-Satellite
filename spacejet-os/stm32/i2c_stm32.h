#ifndef I2C_STM32_H
#define I2C_STM32_H
#include <stdint.h>
void i2c1_init(void);
int  i2c1_write(uint8_t addr7, const uint8_t *data, uint32_t len);
int  i2c1_read (uint8_t addr7, uint8_t *buf,        uint32_t len);
/* Returns 0 on success, -1 on error/timeout */
#endif
