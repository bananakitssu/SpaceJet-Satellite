#ifndef CRC_H
#define CRC_H
#include <stdint.h>
uint16_t crc16_ccitt(const uint8_t *data, uint32_t len);
#endif
