#ifndef TYPES_H
#define TYPES_H

/* These come from the compiler, not libc — safe in -nostdlib */
#include <stdint.h>
#include <stdbool.h>

typedef unsigned int size_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Memory-mapped I/O helpers */
#define MMIO32(addr)   (*(volatile uint32_t *)(uint32_t)(addr))
#define MMIO8(addr)    (*(volatile uint8_t  *)(uint32_t)(addr))

/* Bit helpers */
#define BIT(n)         (1UL << (n))
#define ARRAY_LEN(x)   (sizeof(x) / sizeof((x)[0]))

/* Enable / disable CPU IRQs (ARMv5: no CPS instruction, use MRS/MSR) */
static inline void cpu_irq_enable(void) {
    uint32_t cpsr;
    __asm__ volatile(
        "mrs %0, cpsr\n\t"
        "bic %0, %0, #0x80\n\t"   /* clear I bit */
        "msr cpsr_c, %0\n\t"
        : "=r"(cpsr) :: "memory"
    );
}

static inline void cpu_irq_disable(void) {
    uint32_t cpsr;
    __asm__ volatile(
        "mrs %0, cpsr\n\t"
        "orr %0, %0, #0x80\n\t"   /* set I bit */
        "msr cpsr_c, %0\n\t"
        : "=r"(cpsr) :: "memory"
    );
}

/* Busy-wait loop (compiler barrier prevents optimisation away) */
static inline void nop_delay(volatile uint32_t n) {
    while (n--) __asm__ volatile("nop");
}

#endif /* TYPES_H */
