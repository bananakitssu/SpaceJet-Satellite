/*
 * SpaceJet OS — Cortex-M4 Startup (STM32F405)
 *
 * Cortex-M vector table: word 0 = initial MSP, word 1 = reset address,
 * then exception/IRQ handlers.  All are Thumb-2 (THUMB bit must be set
 * in addresses — linker does this automatically for .thumb_func symbols).
 *
 * CRITICAL DIFFERENCES from ARM926 (versatilepb):
 *   - No ARM mode, only Thumb-2
 *   - Hardware auto-saves r0-r3, r12, lr, pc, xpsr on exception entry
 *   - PendSV exception handles context switching (not IRQ mode switching)
 *   - MSP = handler stack, PSP = task stack (RTOS standard)
 */

.syntax unified
.thumb

/* ── Vector table — must be at Flash base 0x08000000 ── */
.section .vectors, "ax"
.word  _stack_top              /* 00: Initial MSP value              */
.word  reset_handler + 1       /* 04: Reset  (+1 = Thumb addr)       */
.word  fault_handler + 1       /* 08: NMI                            */
.word  fault_handler + 1       /* 0C: HardFault                      */
.word  fault_handler + 1       /* 10: MemManage                      */
.word  fault_handler + 1       /* 14: BusFault                       */
.word  fault_handler + 1       /* 18: UsageFault                     */
.word  0, 0, 0, 0              /* 1C-28: Reserved                    */
.word  fault_handler + 1       /* 2C: SVCall                         */
.word  fault_handler + 1       /* 30: DebugMon                       */
.word  0                       /* 34: Reserved                       */
.word  PendSV_Handler + 1      /* 38: PendSV ← CONTEXT SWITCH        */
.word  SysTick_Handler + 1     /* 3C: SysTick ← sets PendSV each tick*/
/* External IRQs 0..81 — fill with default handler */
.set   irq_count, 82
.rept  irq_count
.word  fault_handler + 1
.endr

/* ── Reset handler ── */
.section .text
.thumb_func
.global reset_handler
reset_handler:
    /* Copy .data from Flash (LMA) to SRAM (VMA) */
    ldr   r0, =_data_start
    ldr   r1, =_data_end
    ldr   r2, =_data_load
.Lcopy:
    cmp   r0, r1
    bge   .Lzero
    ldr   r3, [r2], #4
    str   r3, [r0], #4
    b     .Lcopy
    /* Zero .bss */
.Lzero:
    ldr   r0, =_bss_start
    ldr   r1, =_bss_end
    mov   r2, #0
.Lclr:
    cmp   r0, r1
    bge   .Ldone
    str   r2, [r0], #4
    b     .Lclr
.Ldone:
    bl    main
    b     .

/* ── SysTick handler — calls timer_clear_irq() which sets PendSV ── */
.thumb_func
.global SysTick_Handler
SysTick_Handler:
    push  {r0-r3, lr}
    bl    timer_clear_irq     /* increments g_ticks + sets PENDSVSET */
    pop   {r0-r3, pc}

/* ── PendSV handler — the actual preemptive context switch ── */
/* On entry: hardware has pushed {r0-r3,r12,lr,pc,xpsr} onto PSP */
.thumb_func
.global PendSV_Handler
PendSV_Handler:
    cpsid i                   /* disable interrupts during switch     */
    mrs   r0, psp             /* r0 = current task PSP               */
    cbz   r0, .Lfirst         /* first switch: no task to save        */
    stmdb r0!, {r4-r11}       /* save r4-r11 below hardware frame     */
.Lfirst:
    /* pendsv_schedule(old_psp) → returns new task PSP in r0 */
    bl    pendsv_schedule
    /* r0 = new task PSP (r4-r11 at top, then hardware frame above) */
    ldmia r0!, {r4-r11}       /* restore new task r4-r11              */
    msr   psp, r0             /* update PSP to new task's frame       */
    cpsie i
    /* EXC_RETURN 0xFFFFFFFD = return to Thread mode using PSP        */
    mov   lr, #0xFFFFFFFD
    orr   lr, lr, #0          /* ensure Thumb bit clear in EXC_RETURN */
    bx    lr

/* ── Default fault handler ── */
.thumb_func
.global fault_handler
fault_handler:
    b     fault_handler
