#ifndef SATKEY_H
#define SATKEY_H
#include <stdint.h>
#include <stdbool.h>

#define SATKEY_PATH   "/_credentials/_encrypted_data/satkey"
#define SATKEY_BYTES  16          /* 16 raw bytes = 32 hex chars to type */

/*
 * On first boot: generate a random key, encrypt, store in SJFS.
 * The plaintext key is shown ONCE on the console — record it.
 * Auth: operator provides hex key via  "auth <32-hex-chars>"
 *
 * Encryption: XOR with firmware key (rotating 16-byte sequence).
 * Real satellite → use AES-256-GCM.
 */

void satkey_init(void);                         /* call after sjfs_init()   */
bool satkey_verify(const char *hex_key);        /* verify operator input    */
void satkey_print_path(void);                   /* show storage path        */

/* Session auth (in-RAM flag, cleared on reboot) */
extern bool g_authenticated;

#endif
