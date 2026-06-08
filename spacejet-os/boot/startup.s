/*
 * SpaceJet OS — ARM926EJ-S Startup + IRQ Handler
 * Target: QEMU versatilepb  (loads + executes from 0x00000000)
 *
 * ARM exception vector table MUST live at 0x00000000.
 * Each entry uses "ldr pc, =label" (PC-relative literal load)
 * so it can reach anywhere in the 4GB address space.
 */

.section .vectors, "ax"
.arm
.global _start

_start:
    ldr  pc, =reset_handler          /* 0x00  Reset              */
    ldr  pc, =undef_handler          /* 0x04  Undefined instr    */
    ldr  pc, =swi_handler            /* 0x08  Software interrupt */
    ldr  pc, =prefetch_handler       /* 0x0C  Prefetch abort     */
    ldr  pc, =data_abort_handler     /* 0x10  Data abort         */
    nop                              /* 0x14  Reserved           */
    ldr  pc, =irq_entry              /* 0x18  IRQ                */
    ldr  pc, =fiq_handler            /* 0x1C  FIQ                */

/* Literal pool for the vector table LDRs — must be close (< 4 KB) */
.ltorg

/* ─────────────────────────────────────────────────────────
 * IRQ Entry — saves full task context, calls C dispatcher,
 *             restores context and returns from exception.
 *
 * On entry (IRQ mode):
 *   LR_irq = PC_interrupted + 4   (for normal instruction)
 *   SPSR_irq = CPSR_interrupted
 *   r0-r12  = interrupted task's registers (shared with SYS)
 *
 * We use a dedicated IRQ stack (set up in reset_handler).
 * ───────────────────────────────────────────────────────── */
.section .text.irq
.arm
.global irq_entry

irq_entry:
    sub  lr, lr, #4                  /* Adjust LR: correct return address    */
    stmfd sp!, {r0-r12, lr}         /* Save r0-r12 + return addr on IRQ stk */
    mrs  r0, spsr
    stmfd sp!, {r0}                  /* Save interrupted CPSR on IRQ stack   */

    bl   irq_dispatch                /* C function in vic.c                  */

    ldmfd sp!, {r0}                  /* Restore CPSR into r0                 */
    msr  spsr_cxsf, r0               /* Restore SPSR_irq                     */
    ldmfd sp!, {r0-r12, pc}^        /* Restore regs, return; ^ = CPSR←SPSR */

/* ─────────────────────────────────────────────────────────
 * Reset Handler — sets up per-mode stacks, clears BSS,
 *                 then calls C main().
 * ───────────────────────────────────────────────────────── */
.section .text
.arm
.global reset_handler

reset_handler:
    /*
     * CPSR mode bits [4:0]:  FIQ=0x11  IRQ=0x12  SVC=0x13
     *                        ABT=0x17  UND=0x1B  SYS=0x1F
     * Bit 7 (I) = 1 → IRQ disabled
     * Bit 6 (F) = 1 → FIQ disabled
     * 0xD_ = 1101xxxx = both disabled
     */

    /* FIQ mode: set stack */
    msr  cpsr_c, #0xD1
    ldr  sp, =_fiq_stack_top

    /* IRQ mode: set stack */
    msr  cpsr_c, #0xD2
    ldr  sp, =_irq_stack_top

    /* System mode (same register bank as User): set main stack */
    msr  cpsr_c, #0xDF
    ldr  sp, =_sys_stack_top

    /* Zero the BSS section */
    ldr  r0, =_bss_start
    ldr  r1, =_bss_end
    mov  r2, #0
.Lclear_bss:
    cmp  r0, r1
    strlt r2, [r0], #4
    blt  .Lclear_bss

    /* Hand off to C */
    bl   main

.Lhalt:
    b    .Lhalt                      /* Should never return */

/* ── Default exception stubs ── */
undef_handler:
    b undef_handler
swi_handler:
    b swi_handler
prefetch_handler:
    b prefetch_handler
data_abort_handler:
    b data_abort_handler
fiq_handler:
    b fiq_handler
