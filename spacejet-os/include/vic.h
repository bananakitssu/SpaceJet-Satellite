#ifndef VIC_H
#define VIC_H
#include <stdint.h>
#define VIC_IRQ_TIMER0   4    /* SP804 Timer0/1 → VIC source 4 */
void     vic_init(void);
void     vic_enable_irq(int src);
void     vic_disable_irq(int src);
void     irq_dispatch(void);  /* called from startup.s irq_entry */
#endif
