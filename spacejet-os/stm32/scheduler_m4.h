#ifndef SCHEDULER_M4_H
#define SCHEDULER_M4_H
#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS        8
#define TASK_STACK_WORDS 512    /* 2 KB per task */
#define TASK_NAME_LEN    16

typedef enum { TASK_DEAD=0, TASK_READY, TASK_RUNNING, TASK_SLEEPING } TaskState;

typedef struct {
    uint32_t  *sp;                         /* saved PSP — MUST be first */
    TaskState  state;
    uint32_t   id;
    char       name[TASK_NAME_LEN];
    uint32_t   sleep_until;
    void     (*entry)(void);
    uint32_t   stack[TASK_STACK_WORDS];
} Task;

/* Called from PendSV_Handler assembly — switches tasks, returns new PSP */
uint32_t *pendsv_schedule(uint32_t *old_psp);

int         task_create(const char *name, void (*entry)(void));
void        scheduler_start(void);    /* never returns */
void        scheduler_yield(void);    /* cooperative yield (also used in idle) */
void        task_sleep(uint32_t ticks);
void        task_sleep_ms(uint32_t ms);
void        scheduler_print_tasks(void);

extern volatile uint32_t g_ticks;    /* from systick.c */

#endif
