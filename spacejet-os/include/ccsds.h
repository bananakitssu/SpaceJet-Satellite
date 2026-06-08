#ifndef CCSDS_H
#define CCSDS_H
#include <stdint.h>

/*
 * CCSDS Space Packet Primary Header (6 bytes):
 *   Bits 15-13 : Version (000)
 *   Bit  12    : Packet Type  (0=TM, 1=TC)
 *   Bit  11    : Secondary Header Flag
 *   Bits 10- 0 : APID (Application Process ID)
 *   Bits 15-14 : Sequence Flags (11=standalone)
 *   Bits 13- 0 : Sequence Count
 *   Bits 15- 0 : Packet Data Length (total - 7)
 *
 * Secondary Header (8 bytes, always present here):
 *   4 bytes coarse time (g_ticks)
 *   2 bytes fine time  (sub-tick)
 *   1 byte  APID subtype
 *   1 byte  reserved
 *
 * Packet: [Primary(6)] [Secondary(8)] [Data(N)] [CRC-16(2)]
 */

#define CCSDS_APID_TLM   0x001   /* Housekeeping telemetry */
#define CCSDS_APID_LOG   0x002   /* Event/log packet       */
#define CCSDS_APID_CMD   0x003   /* Command response       */

#define CCSDS_MAX_DATA   256     /* max user data bytes    */

typedef struct __attribute__((packed)) {
    /* Primary header */
    uint16_t  ph_word0;          /* version|type|sechdr|apid  */
    uint16_t  ph_seq;            /* seq_flags|seq_count        */
    uint16_t  ph_data_len;       /* total_len - 7              */
    /* Secondary header */
    uint32_t  sh_coarse;         /* coarse time (g_ticks)      */
    uint16_t  sh_fine;           /* fine time                  */
    uint8_t   sh_subtype;
    uint8_t   sh_reserved;
    /* Data */
    uint8_t   data[CCSDS_MAX_DATA];
    uint16_t  crc;
} CcsdsPacket;

void ccsds_build(CcsdsPacket *pkt, uint16_t apid, uint8_t subtype,
                 const uint8_t *payload, uint16_t payload_len);
void ccsds_send(const CcsdsPacket *pkt);   /* sends over UART as hex dump */
uint16_t ccsds_packet_len(const CcsdsPacket *pkt);

#endif
