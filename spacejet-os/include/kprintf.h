#ifndef KPRINTF_H
#define KPRINTF_H
#include <stdarg.h>
/* Supported: %d %u %x %s %c %p %%  and width/zero-pad e.g. %08x */
void kprintf(const char *fmt, ...);
int  kstrcmp(const char *a, const char *b);
int  kstrlen(const char *s);
#endif
