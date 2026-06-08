/*
 * SpaceJet OS — PL190 VIC Driver
 * Base address: 0x10140000  (versatilepb)
 */
#include "vic.h"
#include "timer.h"
#include "types.h"

#define VIC_BASE        0x10140000U
#define VIC_IRQ_STATUS  MMIO32(VIC_BASE + 0x000)  /* active IRQ sources    */
#define VIC_INT_ENABLE  MMIO32(VIC_BASE + 0x010)  /* enable mask (write 1) */
#define VIC_INT_CLEAR   MMIO32(VIC_BASE + 0x014)  /* clear mask  (write 1) */
#define VIC_VECT_ADDR   MMIO32(VIC_BASE + 0x030)  /* current vector / EOI  */

void vic_init(void) {
    VIC_INT_CLEAR = 0xFFFFFFFFU;   /* disable every source */
    VIC_VECT_ADDR = 0;             /* clear any pending EOI */
}

void vic_enable_irq(int src) {
    VIC_INT_ENABLE = BIT(src);
}

void vic_disable_irq(int src) {
    VIC_INT_CLEAR = BIT(src);
}

/* Called from irq_entry (startup.s) after context save */
void irq_dispatch(void) {
    uint32_t status = VIC_IRQ_STATUS;
    if (status & BIT(VIC_IRQ_TIMER0)) {
        timer_clear_irq();          /* increment g_ticks, clear SP804 flag */
    }
    VIC_VECT_ADDR = 0;              /* signal EOI to VIC */
}
