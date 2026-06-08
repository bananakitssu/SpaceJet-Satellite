#include "satkey.h"
#include "sjfs.h"
#include "kstring.h"
#include "kprintf.h"
#include "timer.h"
#include <stdint.h>
#include <stdbool.h>

bool g_authenticated = false;

/* Firmware-embedded obfuscation key (XOR).
 * Change this before building for your satellite. */
static const uint8_t FW_KEY[16] = {
    0x53,0x4A,0x4F,0x53,0x2D,0x53,0x41,0x54,
    0x2D,0x4B,0x45,0x59,0x2D,0x56,0x31,0x00
};  /* "SJOS-SAT-KEY-V1\0" */

/* Stored file format: [4-byte magic "SJKY"] [16 XOR-encrypted bytes] */
#define SK_MAGIC 0x534A4B59U   /* "SJKY" */

static void xor_key(const uint8_t *in, uint8_t *out, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        out[i] = in[i] ^ FW_KEY[i % 16];
}

/* Simple LCG PRNG seeded with tick counter + some constants */
static uint8_t prng_next(uint32_t *state) {
    *state = (*state) * 1664525U + 1013904223U;
    return (uint8_t)(*state >> 24);
}

void satkey_init(void) {
    int idx = sjfs_open(SATKEY_PATH);
    if (idx >= 0) {
        kprintf("  [KEY] SatKey loaded from %s\n", SATKEY_PATH);
        return;   /* already exists — do not regenerate */
    }

    /* First boot: generate and store */
    kprintf("  [KEY] First boot — generating SatKey...\n");

    uint8_t plain[SATKEY_BYTES];
    uint32_t seed = g_ticks ^ 0xDEADBEEFU;
    for (uint32_t i = 0; i < SATKEY_BYTES; i++)
        plain[i] = prng_next(&seed);

    /* Encrypt */
    uint8_t enc[SATKEY_BYTES];
    xor_key(plain, enc, SATKEY_BYTES);

    /* Write to SJFS: magic + encrypted key */
    idx = sjfs_create(SATKEY_PATH);
    uint32_t magic = SK_MAGIC;
    sjfs_write(idx, (uint8_t *)&magic, 4);
    sjfs_write(idx, enc, SATKEY_BYTES);

    /* Show key ONCE — operator must record this */
    char hexbuf[SATKEY_BYTES * 2 + 1];
    bytes_to_hex(plain, SATKEY_BYTES, hexbuf);

    kprintf("\n");
    kprintf("  ╔══════════════════════════════════════════════╗\n");
    kprintf("  ║          *** SATELLITE AUTH KEY ***          ║\n");
    kprintf("  ║   Record this — it will NOT be shown again   ║\n");
    kprintf("  ║                                              ║\n");
    kprintf("  ║   SatKey: %s  ║\n", hexbuf);
    kprintf("  ║                                              ║\n");
    kprintf("  ║   Stored (encrypted) at:                    ║\n");
    kprintf("  ║   %-44s║\n", SATKEY_PATH);
    kprintf("  ╚══════════════════════════════════════════════╝\n");
    kprintf("\n");
}

bool satkey_verify(const char *hex_key) {
    if (!hex_key || kstrlen(hex_key) < SATKEY_BYTES * 2) return false;

    /* Read from SJFS */
    int idx = sjfs_open(SATKEY_PATH);
    if (idx < 0) return false;

    uint8_t stored[4 + SATKEY_BYTES];
    uint32_t got = 0;
    sjfs_read(idx, stored, sizeof(stored), &got);
    if (got < 4 + SATKEY_BYTES) return false;

    /* Check magic */
    uint32_t magic;
    kmemcpy(&magic, stored, 4);
    if (magic != SK_MAGIC) return false;

    /* Decrypt */
    uint8_t plain[SATKEY_BYTES];
    xor_key(&stored[4], plain, SATKEY_BYTES);

    /* Compare against provided hex */
    uint8_t provided[SATKEY_BYTES];
    if (hex_to_bytes(hex_key, provided, SATKEY_BYTES) != 0) return false;

    return kmemcmp(plain, provided, SATKEY_BYTES) == 0;
}

void satkey_print_path(void) {
    kprintf("  SatKey path : %s\n", SATKEY_PATH);
    kprintf("  Auth status : %s\n", g_authenticated ? "AUTHENTICATED" : "NOT AUTH");
}
