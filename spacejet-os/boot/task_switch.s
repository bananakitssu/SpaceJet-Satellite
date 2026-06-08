/*
 * SpaceJet OS — Cooperative Context Switch
 *
 * void task_switch(uint32_t **old_sp, uint32_t *new_sp)
 *   r0 = address of old task's saved-SP field  (&task->sp)
 *   r1 = new task's saved stack pointer         (task->sp)
 *
 * Saves {r4-r11, lr} (callee-saved + return addr) on the old
 * task's stack, then switches to the new task's stack and pops
 * {r4-r11, pc} to resume exactly where it yielded.
 *
 * New tasks are initialised with a fake saved frame where the
 * "saved lr" (→ pc on pop) is the task entry function.
 */

.section .text
.arm
.global task_switch

task_switch:
    stmfd sp!, {r4-r11, lr}     /* push callee-saved regs + return addr */
    str   sp, [r0]              /* *old_sp = sp  (save old stack ptr)   */
    mov   sp, r1                /* sp = new_sp   (switch to new task)   */
    ldmfd sp!, {r4-r11, pc}    /* pop + jump to new task               */
