/*
 * SpaceJet OS — STM32F405RG Linker Script
 * Flash: 0x08000000  1 MB
 * SRAM1: 0x20000000  112 KB (main RAM)
 * SRAM2: 0x2001C000  16 KB
 * CCM  : 0x10000000  64 KB (Core Coupled Memory — no DMA access)
 *
 * SJFS lives at top of SRAM1 (highest 256 KB):
 *   0x20000000 + (112*1024 - 4*1024) = 0x2001B000
 *   Use only 4 KB here — real satellite uses external SPI flash.
 */
ENTRY(reset_handler)

MEMORY {
    FLASH  (rx)  : ORIGIN = 0x08000000, LENGTH = 1M
    SRAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 112K
    CCM    (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

_sys_stack_size = 0x2000;   /* 8 KB MSP stack */

SECTIONS {
    .vectors : { KEEP(*(.vectors)) } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
    } > FLASH

    . = ALIGN(4);
    _data_load = .;          /* LMA — where .data lives in Flash     */

    .data : AT(_data_load) {
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > SRAM

    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > SRAM

    /* Main/MSP stack at end of SRAM */
    . = ALIGN(8);
    . = . + _sys_stack_size;
    _stack_top = .;
}
