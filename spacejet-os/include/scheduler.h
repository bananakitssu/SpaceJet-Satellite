#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "task.h"

/* Implemented in boot/task_switch.s */
void task_switch(uint32_t **old_sp, uint32_t *new_sp);

int         task_create(const char *name, void (*entry)(void));
void        scheduler_start(void);       /* enters first task — never returns */
void        scheduler_yield(void);       /* voluntarily release the CPU       */
void        task_sleep(uint32_t ticks);  /* sleep N ticks then auto-wake      */
void        task_sleep_ms(uint32_t ms);  /* convenience wrapper               */
void        scheduler_print_tasks(void); /* for 'tasks' shell command         */
const char *scheduler_task_name(int id);

#endif
void preempt_check(void);   /* call at safe yield points inside tasks */
