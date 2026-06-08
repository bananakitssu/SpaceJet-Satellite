/*
 * SpaceJet OS Linker Script
 * Target: ARM926EJ-S — QEMU versatilepb
 * Vectors must be at 0x00000000 (ARM9 hardware requirement)
 */
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 256M
}

_irq_stack_size = 0x400;    /* 1 KB  — IRQ exception stack  */
_fiq_stack_size = 0x100;    /* 256 B — FIQ exception stack  */
_sys_stack_size = 0x8000;   /* 32 KB — main / task stack    */

SECTIONS {
    . = 0x00000000;

    /* ARM exception vector table — MUST start at address 0 */
    .vectors : {
        KEEP(*(.vectors))
    } > RAM

    .text : {
        *(.text.irq)        /* IRQ entry right after vectors  */
        *(.text*)
        *(.rodata*)
    } > RAM

    . = ALIGN(4);
    .data : {
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > RAM

    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > RAM

    /* Exception stacks (grow downward, placed after BSS) */
    . = ALIGN(8);
    . = . + _fiq_stack_size;
    _fiq_stack_top = .;

    . = ALIGN(8);
    . = . + _irq_stack_size;
    _irq_stack_top = .;

    . = ALIGN(8);
    . = . + _sys_stack_size;
    _sys_stack_top = .;

    _heap_start = .;
}
