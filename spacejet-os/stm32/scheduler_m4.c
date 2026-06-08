/*
 * SpaceJet OS — Cortex-M4 Preemptive Scheduler
 *
 * TRUE PREEMPTION via PendSV:
 *   SysTick (every 10 ms) → sets PENDSVSET
 *   PendSV fires (lowest priority, after all ISRs done) → calls pendsv_schedule()
 *   pendsv_schedule() picks next task → PendSV assembly restores registers
 *
 * Task stack layout when saved (Cortex-M4, PSP descending):
 *   [PSP+0 ] r4      ← pendsv_schedule returns PSP pointing here
 *   [PSP+4 ] r5
 *   ...
 *   [PSP+28] r11
 *   --- hardware auto-frame below (pushed by CPU on exception entry) ---
 *   [PSP+32] r0
 *   [PSP+36] r1
 *   [PSP+40] r2
 *   [PSP+44] r3
 *   [PSP+48] r12
 *   [PSP+52] lr   (EXC_RETURN on first run = 0xFFFFFFFD)
 *   [PSP+56] pc   (task entry on first run)
 *   [PSP+60] xpsr (0x01000000 = Thumb bit set)
 */
#include "scheduler_m4.h"
#include "systick.h"
#include "uart_stm32.h"  /* for kprintf via scheduler_print_tasks */
#include <stdint.h>

/* Forward declare kprintf since we don't include all headers here */
extern void kprintf(const char *fmt, ...);
extern int  kstrcmp(const char *a, const char *b);
extern int  kstrncpy(char *dst, const char *src, unsigned int n);

static Task   s_tasks[MAX_TASKS];
static int    s_count   = 0;
static int    s_current = -1;   /* -1 = not started */

int task_create(const char *name, void (*entry)(void)) {
    if (s_count >= MAX_TASKS) return -1;
    Task *t = &s_tasks[s_count];
    t->id    = (uint32_t)s_count;
    t->state = TASK_READY;
    t->entry = entry;
    t->sleep_until = 0;
    int i = 0;
    while (name[i] && i < TASK_NAME_LEN-1) { t->name[i] = name[i]; i++; }
    t->name[i] = '\0';

    /*
     * Build initial Cortex-M4 stack frame.
     * PSP will point to r4 (top of manually-saved area).
     * Hardware frame sits above (higher addresses).
     */
    uint32_t *sp = &t->stack[TASK_STACK_WORDS];
    /* Hardware frame (pushed by CPU on exception entry, highest addr first) */
    *--sp = 0x01000000U;        /* xpsr: Thumb bit set                    */
    *--sp = (uint32_t)entry;    /* pc:   task entry function               */
    *--sp = 0xFFFFFFFDU;        /* lr:   EXC_RETURN (Thread mode, PSP)    */
    *--sp = 0;                  /* r12                                     */
    *--sp = 0;                  /* r3                                      */
    *--sp = 0;                  /* r2                                      */
    *--sp = 0;                  /* r1                                      */
    *--sp = 0;                  /* r0                                      */
    /* Manually-saved frame (r4-r11, pushed by PendSV stmdb) */
    *--sp = 0;  /* r11 */
    *--sp = 0;  /* r10 */
    *--sp = 0;  /* r9  */
    *--sp = 0;  /* r8  */
    *--sp = 0;  /* r7  */
    *--sp = 0;  /* r6  */
    *--sp = 0;  /* r5  */
    *--sp = 0;  /* r4  */    /* ← PSP points here when restored */

    t->sp = sp;
    return s_count++;
}

/*
 * pendsv_schedule — called from PendSV_Handler assembly.
 * Saves old PSP, picks next task, returns new PSP.
 * Runs with interrupts disabled (cpsid i in asm).
 */
uint32_t *pendsv_schedule(uint32_t *old_psp) {
    /* Save old task's PSP (old_psp already has r4-r11 pushed by asm) */
    if (s_current >= 0) {
        s_tasks[s_current].sp = old_psp;
        if (s_tasks[s_current].state == TASK_RUNNING)
            s_tasks[s_current].state = TASK_READY;
    }

    /* Wake sleeping tasks */
    for (int i = 0; i < s_count; i++) {
        if (s_tasks[i].state == TASK_SLEEPING &&
            g_ticks >= s_tasks[i].sleep_until)
            s_tasks[i].state = TASK_READY;
    }

    /* Round-robin: find next READY task */
    int next = (s_current < 0) ? 0 : s_current;
    for (int i = 0; i < s_count; i++) {
        int n = (next + 1 + i) % s_count;
        if (s_tasks[n].state == TASK_READY) { next = n; break; }
    }

    s_tasks[next].state = TASK_RUNNING;
    s_current = next;
    return s_tasks[next].sp;
}

void scheduler_start(void) {
    if (s_count == 0) return;
    /*
     * Switch CPU to use PSP for Thread mode.
     * After this, MSP is reserved for exceptions only.
     * Then trigger PendSV to start first task.
     */
    __asm__ volatile(
        "msr psp, %0\n\t"       /* set PSP (doesn't matter, PendSV will set it) */
        "mov r0, #2\n\t"        /* CONTROL: use PSP in Thread mode              */
        "msr control, r0\n\t"
        "isb\n\t"               /* instruction sync barrier                      */
        ::"r"(s_tasks[0].sp) : "r0"
    );
    /* Trigger first PendSV */
    extern void SCB_ICSR_set(void);
    /* Inline: set PENDSVSET */
    volatile uint32_t *icsr = (volatile uint32_t *)0xE000ED04U;
    *icsr |= (1U << 28);
    /* Enable interrupts — PendSV will fire and start first task */
    __asm__ volatile("cpsie i");
    /* Spin here — PendSV takes over */
    while (1) __asm__("wfi");
}

void scheduler_yield(void) {
    /* Trigger PendSV — same mechanism as preemption, just software-initiated */
    volatile uint32_t *icsr = (volatile uint32_t *)0xE000ED04U;
    *icsr |= (1U << 28);
    __asm__ volatile("isb");
}

void task_sleep(uint32_t ticks) {
    s_tasks[s_current].sleep_until = g_ticks + ticks;
    s_tasks[s_current].state       = TASK_SLEEPING;
    scheduler_yield();
}

void task_sleep_ms(uint32_t ms) { task_sleep((ms + 9) / 10); }

void scheduler_print_tasks(void) {
    static const char *sn[] = {"DEAD","READY","RUNNING","SLEEPING"};
    kprintf("  ID  Name             State\n");
    kprintf("  --  ---------------  --------\n");
    for (int i = 0; i < s_count; i++)
        kprintf("  %2u  %-15s  %s\n", s_tasks[i].id, s_tasks[i].name,
                s_tasks[i].state < 4 ? sn[s_tasks[i].state] : "?");
}
