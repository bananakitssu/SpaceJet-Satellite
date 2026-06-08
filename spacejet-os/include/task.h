#ifndef TASK_H
#define TASK_H
#include <stdint.h>

#define MAX_TASKS        8
#define TASK_STACK_WORDS 512     /* 2 KB stack per task */
#define TASK_NAME_LEN    16

typedef enum {
    TASK_DEAD     = 0,
    TASK_READY    = 1,
    TASK_RUNNING  = 2,
    TASK_SLEEPING = 3,
} TaskState;

typedef struct {
    uint32_t  *sp;                        /* saved SP — must be first field */
    TaskState  state;
    uint32_t   id;
    char       name[TASK_NAME_LEN];
    uint32_t   sleep_until;              /* g_ticks value to wake at       */
    void     (*entry)(void);
    uint32_t   stack[TASK_STACK_WORDS];  /* private stack                   */
} Task;

#endif
