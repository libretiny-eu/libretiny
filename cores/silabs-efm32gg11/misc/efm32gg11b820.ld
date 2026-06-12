/* Phase 1 linker script for EFM32GG11B820F2048GM64.
 * Single flash image at 0x00000000 (no bootloader).
 * Vector table at start of flash. Heap above .bss; stack at top of RAM.
 *
 * The vendor startup file (Source/GCC/startup_efm32gg11b.S) initialises .data
 * and .bss via CMSIS-style table-driven loops in Reset_Handler, indexing
 * __copy_table_{start,end}__ and __zero_table_{start,end}__. Those symbols
 * MUST be provided here even if our project happens to have no .data — the
 * startup code unconditionally walks the tables. Empty tables are valid.
 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 2048K
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
}

ENTRY(Reset_Handler)

_stack_size = 4096;   /* main stack (interrupt context); FreeRTOS task stacks
                       * are allocated separately from the FreeRTOS heap. */

SECTIONS
{
    .text : {
        /* The vendor startup_efm32gg11b.S declares `.section .vectors` with no
         * flags, so that input section is non-ALLOC. It must live inside an
         * ALLOC output section to be placed at all — folding it into .text (as
         * the vendor efm32gg11b.ld does) puts __Vectors at 0x0. A separate
         * output section inherited the non-ALLOC flag and got overlapped. */
        KEEP(*(.vectors))
        KEEP(*(.isr_vector))   /* kept for portability; empty with the vendor startup */
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
    } > FLASH

    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* CMSIS copy table: triplets of (src, dst, word_count) used by Reset_Handler
     * to initialise .data from flash. We have one .data section so one entry. */
    .copy.table : {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__etext)
        LONG (__data_start__)
        LONG ((__data_end__ - __data_start__) / 4)
        __copy_table_end__ = .;
    } > FLASH

    /* CMSIS zero table: pairs of (start, BYTE count). Asymmetric with the copy
     * table on purpose: the startup zero loop consumes the count directly as
     * bytes (no `lsls #2`), unlike the copy loop which takes words. The vendor
     * .ld leaves this table empty and lets newlib crt0 zero the main .bss, but
     * we redirect __START to lt_main (no crt0), so this entry is the ONLY .bss
     * zeroing — a word count here leaves 3/4 of .bss as power-on garbage. */
    .zero.table : {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        __zero_table_end__ = .;
    } > FLASH

    __etext = ALIGN(4);

    .data : AT (__etext) {
        __data_start__ = .;
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);
        _edata = .;
        __data_end__ = .;
    } > RAM

    _sidata = LOADADDR(.data);   /* compat alias; vendor startup uses __etext */

    .bss : {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = .;
    } > RAM

    .heap (COPY) : {
        __end__ = .;
        end = __end__;
        KEEP(*(.heap*))
        __HeapLimit = ORIGIN(RAM) + LENGTH(RAM) - _stack_size;
    } > RAM

    .stack_dummy (COPY) : {
        KEEP(*(.stack*))
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
    _sstack = _estack - _stack_size;
    PROVIDE(_StackTop = _estack);

    ASSERT(__HeapLimit >= __bss_end__, "region RAM overflowed with heap")
}
