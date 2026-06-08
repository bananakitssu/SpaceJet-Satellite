#ifndef KSTRING_H
#define KSTRING_H
#include <stdint.h>
void     *kmemset(void *dst, int c, uint32_t n);
void     *kmemcpy(void *dst, const void *src, uint32_t n);
int       kmemcmp(const void *a, const void *b, uint32_t n);
char     *kstrcpy(char *dst, const char *src);
char     *kstrncpy(char *dst, const char *src, uint32_t n);
char     *kstrcat(char *dst, const char *src);
int       kstrcmp(const char *a, const char *b);
int       kstrlen(const char *s);
void      bytes_to_hex(const uint8_t *b, uint32_t len, char *out);
int       hex_to_bytes(const char *hex, uint8_t *out, uint32_t len);
/* Minimal buffer sprintf — supports %u %d %s %c %02u style */
int       ksnprintf(char *buf, uint32_t size, const char *fmt, ...);
#endif
