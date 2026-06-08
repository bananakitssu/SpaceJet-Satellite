/*
 * SpaceJet OS — Cooperative Round-Robin Scheduler
 *
 * Tasks call scheduler_yield() to voluntarily give up the CPU.
 * The SP804 timer ISR increments g_ticks so task_sleep() works
 * without needing a preemptive context switch in the ISR.
 *
 * task_switch() (assembly) saves {r4-r11, lr} on the outgoing
 * task's stack, stores the SP, loads the incoming SP, then pops
 * {r4-r11, pc} — resuming exactly where that task yielded.
 */
#include "scheduler.h"
#include "timer.h"
#include "types.h"
#include "kprintf.h"

static Task   s_tasks[MAX_TASKS];
static int    s_count   = 0;
static int    s_current = 0;

/* Scratch SP for the very first task_switch (never restored to) */
static uint32_t  s_init_dummy = 0;
static uint32_t *s_init_sp    = &s_init_dummy;

int task_create(const char *name, void (*entry)(void)) {
    if (s_count >= MAX_TASKS) return -1;

    Task *t    = &s_tasks[s_count];
    t->id      = (uint32_t)s_count;
    t->state   = TASK_READY;
    t->entry   = entry;
    t->sleep_until = 0;

    int i = 0;
    while (name[i] && i < TASK_NAME_LEN - 1) { t->name[i] = name[i]; i++; }
    t->name[i] = '\0';

    /*
     * Build an initial stack frame identical to what task_switch()
     * produces with stmfd sp!, {r4-r11, lr}.
     * Layout at task->sp (lowest addr = top of descending stack):
     *   [sp+0]  r4  = 0
     *   [sp+4]  r5  = 0  ...
     *   [sp+28] r11 = 0
     *   [sp+32] lr  = entry   ← becomes PC on first ldmfd …, {r4-r11, pc}
     */
    uint32_t *sp = &t->stack[TASK_STACK_WORDS];
    *--sp = (uint32_t)entry;  /* lr → pc */
    *--sp = 0;  /* r11 */
    *--sp = 0;  /* r10 */
    *--sp = 0;  /* r9  */
    *--sp = 0;  /* r8  */
    *--sp = 0;  /* r7  */
    *--sp = 0;  /* r6  */
    *--sp = 0;  /* r5  */
    *--sp = 0;  /* r4  */
    t->sp = sp;

    return s_count++;
}

void scheduler_start(void) {
    if (s_count == 0) return;
    s_current = 0;
    s_tasks[0].state = TASK_RUNNING;
    /* Switch from bootstrap context to task 0 (s_init_sp never restored) */
    task_switch(&s_init_sp, s_tasks[0].sp);
    /* Never reached */
}

void scheduler_yield(void) {
    int old  = s_current;
    int next = -1;

    /* Wake any tasks whose sleep timer expired */
    for (int i = 0; i < s_count; i++) {
        if (s_tasks[i].state == TASK_SLEEPING &&
            g_ticks >= s_tasks[i].sleep_until)
            s_tasks[i].state = TASK_READY;
    }

    /* Mark current task ready (unless it explicitly went to sleep) */
    if (s_tasks[old].state == TASK_RUNNING)
        s_tasks[old].state = TASK_READY;

    /* Round-robin: find next READY task after current */
    for (int i = 0; i < s_count; i++) {
        int n = (old + 1 + i) % s_count;
        if (s_tasks[n].state == TASK_READY) { next = n; break; }
    }

    /* Nothing ready yet — spin until a sleeping task wakes */
    while (next < 0) {
        for (int i = 0; i < s_count; i++) {
            if (s_tasks[i].state == TASK_SLEEPING &&
                g_ticks >= s_tasks[i].sleep_until) {
                s_tasks[i].state = TASK_READY;
                next = i;
                break;
            }
        }
    }

    /* Avoid a needless switch to self */
    if (next == old) { s_tasks[old].state = TASK_RUNNING; return; }

    s_tasks[next].state = TASK_RUNNING;
    s_current = next;
    task_switch(&s_tasks[old].sp, s_tasks[next].sp);
}

void task_sleep(uint32_t ticks) {
    s_tasks[s_current].sleep_until = g_ticks + ticks;
    s_tasks[s_current].state       = TASK_SLEEPING;
    scheduler_yield();
}

void task_sleep_ms(uint32_t ms) {
    /* TICK_HZ = 100 → 10 ms per tick; round up */
    task_sleep((ms + 9) / 10);
}

void scheduler_print_tasks(void) {
    static const char *snames[] = {"DEAD","READY","RUNNING","SLEEPING"};
    kprintf("  ID  Name             State\n");
    kprintf("  --  ---------------  --------\n");
    for (int i = 0; i < s_count; i++) {
        Task *t = &s_tasks[i];
        kprintf("  %2u  %-15s  %s\n",
            t->id, t->name,
            t->state < 4 ? snames[t->state] : "?");
    }
}

const char *scheduler_task_name(int id) {
    if (id < 0 || id >= s_count) return "?";
    return s_tasks[id].name;
}

/* ── Soft preemption (v2 addition) ── */
extern volatile uint8_t g_preempt_request;

void preempt_check(void) {
    if (g_preempt_request) {
        g_preempt_request = 0;
        scheduler_yield();
    }
}
