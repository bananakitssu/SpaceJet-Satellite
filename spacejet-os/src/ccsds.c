#include "ccsds.h"
#include "crc.h"
#include "uart.h"
#include "timer.h"
#include "kstring.h"
#include "kprintf.h"
#include <stdint.h>

static uint16_t s_seq_count = 0;

void ccsds_build(CcsdsPacket *pkt, uint16_t apid, uint8_t subtype,
                 const uint8_t *payload, uint16_t payload_len) {
    if (payload_len > CCSDS_MAX_DATA) payload_len = CCSDS_MAX_DATA;

    /* Primary header word 0: version=0, type=TM(0), sechdr=1, apid */
    pkt->ph_word0   = (uint16_t)(0x0800 | (apid & 0x07FF));  /* sechdr bit set */
    /* Sequence: standalone (0xC000) | count */
    pkt->ph_seq     = (uint16_t)(0xC000 | (s_seq_count++ & 0x3FFF));
    /* Data length = secondary_header(8) + payload + crc(2) - 1 */
    pkt->ph_data_len = (uint16_t)(8 + payload_len + 2 - 1);

    /* Secondary header */
    pkt->sh_coarse   = g_ticks;
    pkt->sh_fine     = 0;
    pkt->sh_subtype  = subtype;
    pkt->sh_reserved = 0;

    /* Copy payload */
    if (payload && payload_len)
        kmemcpy(pkt->data, payload, payload_len);

    /* CRC over header + secondary header + data (everything except crc field) */
    uint16_t covered = (uint16_t)(6 + 8 + payload_len);
    pkt->crc = crc16_ccitt((const uint8_t *)pkt, covered);
}

uint16_t ccsds_packet_len(const CcsdsPacket *pkt) {
    return (uint16_t)(6 + pkt->ph_data_len + 1);  /* CCSDS: total = datalen + 7 */
}

/* Send packet over UART as a framed hex dump
 * Frame: 0x7E <len_hi> <len_lo> <hex_bytes> 0x7E
 * In real flight, this would be binary over RF. */
void ccsds_send(const CcsdsPacket *pkt) {
    uint16_t total = ccsds_packet_len(pkt);
    const uint8_t *raw = (const uint8_t *)pkt;
    char hexbuf[4];

    uart_putc(0x7E);                           /* frame start */
    uart_putc((uint8_t)(total >> 8));
    uart_putc((uint8_t)(total & 0xFF));
    for (uint16_t i = 0; i < total; i++) {
        bytes_to_hex(&raw[i], 1, hexbuf);
        uart_putc(hexbuf[0]);
        uart_putc(hexbuf[1]);
        uart_putc(' ');
    }
    uart_putc(0x7E);                           /* frame end */
    uart_puts("\r\n");
}
