#include "kstring.h"
#include <stdarg.h>
#include <stdint.h>

void *kmemset(void *dst, int c, uint32_t n) {
    uint8_t *p = dst;
    while (n--) *p++ = (uint8_t)c;
    return dst;
}
void *kmemcpy(void *dst, const void *src, uint32_t n) {
    uint8_t *d = dst; const uint8_t *s = src;
    while (n--) *d++ = *s++;
    return dst;
}
int kmemcmp(const void *a, const void *b, uint32_t n) {
    const uint8_t *p = a, *q = b;
    while (n--) { if (*p != *q) return *p - *q; p++; q++; }
    return 0;
}
char *kstrcpy(char *dst, const char *src) {
    char *r = dst; while ((*dst++ = *src++)); return r;
}
char *kstrncpy(char *dst, const char *src, uint32_t n) {
    char *r = dst;
    while (n && (*dst++ = *src++)) n--;
    if (n) *dst = '\0';
    return r;
}
char *kstrcat(char *dst, const char *src) {
    char *r = dst;
    while (*dst) dst++;
    while ((*dst++ = *src++));
    return r;
}
int kstrcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}
int kstrlen(const char *s) { int n=0; while(*s++) n++; return n; }

void bytes_to_hex(const uint8_t *b, uint32_t len, char *out) {
    const char *h = "0123456789abcdef";
    for (uint32_t i = 0; i < len; i++) {
        out[i*2]   = h[b[i] >> 4];
        out[i*2+1] = h[b[i] & 0xF];
    }
    out[len*2] = '\0';
}
int hex_to_bytes(const char *hex, uint8_t *out, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        char hi = hex[i*2], lo = hex[i*2+1];
        if (!hi || !lo) return -1;
        uint8_t hv = (hi>='a') ? hi-'a'+10 : (hi>='A') ? hi-'A'+10 : hi-'0';
        uint8_t lv = (lo>='a') ? lo-'a'+10 : (lo>='A') ? lo-'A'+10 : lo-'0';
        out[i] = (hv << 4) | lv;
    }
    return 0;
}

/* Tiny buffer printf (subset: %u %d %s %c %02u %08x) */
int ksnprintf(char *buf, uint32_t size, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    uint32_t pos = 0;
#define PUTC(c) do { if (pos+1 < size) buf[pos++] = (c); } while(0)
    while (*fmt && pos+1 < size) {
        if (*fmt != '%') { PUTC(*fmt++); continue; }
        fmt++;
        char pad = ' '; int width = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') { width = width*10 + (*fmt-'0'); fmt++; }
        char tmp[32]; int ti = 0;
        switch (*fmt) {
        case 'u': case 'd': {
            uint32_t v; int neg = 0;
            if (*fmt=='d') { int32_t sv = va_arg(ap,int32_t); if(sv<0){neg=1;v=(uint32_t)(-sv);}else v=(uint32_t)sv; }
            else v = va_arg(ap,uint32_t);
            if (!v) tmp[ti++]='0';
            while (v) { tmp[ti++]='0'+(v%10); v/=10; }
            if (neg) tmp[ti++]='-';
            while (ti < width) tmp[ti++] = pad;
            while (ti--) PUTC(tmp[ti]);
            break; }
        case 'x': {
            uint32_t v = va_arg(ap,uint32_t);
            const char *hx = "0123456789abcdef";
            if (!v) tmp[ti++]='0';
            while (v) { tmp[ti++]=hx[v&0xF]; v>>=4; }
            while (ti < width) tmp[ti++] = pad;
            while (ti--) PUTC(tmp[ti]);
            break; }
        case 's': { const char *s=va_arg(ap,const char*); if(!s)s="(null)"; while(*s) PUTC(*s++); break; }
        case 'c': PUTC((char)va_arg(ap,int)); break;
        case '%': PUTC('%'); break;
        default:  PUTC('%'); PUTC(*fmt); break;
        }
        fmt++;
    }
#undef PUTC
    buf[pos] = '\0';
    va_end(ap);
    return (int)pos;
}
