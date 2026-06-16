/* OTA linker script for EFM32GG11B820F2048GM64.
 * A/B dual-bank app: bank A @ 0x008000, bank B @ 0x100000 (each 0x0F0000;
 * the 32 KB below metadata is the `kvs` FlashDB partition).
 * Vector table at start of the bank. Heap above .bss; stack at top of RAM.
 *
 * The FLASH origin/length are parameterized via -Wl,--defsym so the SAME
 * sources can link at either bank base (the builder re-links bank B). The
 * defaults below keep a plain `pio run` linking bank A (origin 0x008000),
 * which is a valid bank-A image. See efm32gg11b820.template.ld (the kept
 * copy of this script) for the bank-B link path.
 *
 * The vendor startup file (Source/GCC/startup_efm32gg11b.S) initialises .data
 * and .bss via CMSIS-style table-driven loops in Reset_Handler, indexing
 * __copy_table_{start,end}__ and __zero_table_{start,end}__. Those symbols
 * MUST be provided here even if our project happens to have no .data — the
 * startup code unconditionally walks the tables. Empty tables are valid.
 */

/* Origin/length injected by the builder via -Wl,--defsym for each bank.
   Defaults keep a plain `pio run` linking bank A. */
__flash_origin = DEFINED(__flash_origin) ? __flash_origin : 0x00008000;
__flash_length = DEFINED(__flash_length) ? __flash_length : 0x000F0000;
MEMORY
{
    FLASH (rx) : ORIGIN = __flash_origin, LENGTH = __flash_length
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
        /* .ram (SL_RAMFUNC code, e.g. emlib MSC flash routines) is copied to RAM
         * before main() too: on EFM32GG11 (Series 1) the MSC write loop MUST
         * execute from RAM, else a sustained flash write hangs mid-transfer. */
        LONG (LOADADDR(.ram))
        LONG (__ram_start__)
        LONG ((__ram_end__ - __ram_start__) / 4)
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

    /* RAM-resident code (SL_RAMFUNC). emlib MSC flash routines land here when
     * EM_MSC_RUN_FROM_RAM is defined; the copy table above copies it to RAM and
     * the startup runs it from there. LMA in FLASH (part of the flash image),
     * VMA in RAM. Always emitted (small/empty if no ramfuncs in a build). */
    .ram : AT (LOADADDR(.data) + SIZEOF(.data)) {
        . = ALIGN(4);
        __ram_start__ = .;
        *(.ram .ram.*)
        . = ALIGN(4);
        __ram_end__ = .;
    } > RAM

    /* Loadable-image body length (text+rodata+data init+ram init), used by
       lt_ota.c to record the RUNNING app's own length into OTA metadata at
       confirm time. .ram is the last flash-LMA section, so it bounds the image. */
    _lt_image_size = LOADADDR(.ram) + SIZEOF(.ram) - ORIGIN(FLASH);

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
