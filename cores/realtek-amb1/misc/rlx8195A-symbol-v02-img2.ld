

ENTRY(Reset_Handler)

/* AddCoreSources builds both amb1 and its amb parent as archives. Child
 * archives precede parents (BuildLibraries prepends LIBS). Force this device
 * before archive scanning so the inherited writable flash0 is never pulled. */
EXTERN(flash0)

/* These undefined references are renamed only when the builder enables the
 * corresponding --wrap flags. Missing protection is a link error, not a
 * warning that can be missed before the first bench boot. */
EXTERN(__wrap_flash_write_word __wrap_flash_stream_write __wrap_flash_burst_write)
EXTERN(__wrap_flash_erase_sector __wrap_flash_erase_block __wrap_flash_erase_chip)
EXTERN(__wrap_flash_write_protect __wrap_flash_set_status __wrap_flash_reset_status)
EXTERN(__wrap_flash_set_extend_addr)
ASSERT(flash_write_word == __wrap_flash_write_word, "AMB1 requires --wrap=flash_write_word")
ASSERT(flash_stream_write == __wrap_flash_stream_write, "AMB1 requires --wrap=flash_stream_write")
ASSERT(flash_burst_write == __wrap_flash_burst_write, "AMB1 requires --wrap=flash_burst_write")
ASSERT(flash_erase_sector == __wrap_flash_erase_sector, "AMB1 requires --wrap=flash_erase_sector")
ASSERT(flash_erase_block == __wrap_flash_erase_block, "AMB1 requires --wrap=flash_erase_block")
ASSERT(flash_erase_chip == __wrap_flash_erase_chip, "AMB1 requires --wrap=flash_erase_chip")
ASSERT(flash_write_protect == __wrap_flash_write_protect, "AMB1 requires --wrap=flash_write_protect")
ASSERT(flash_set_status == __wrap_flash_set_status, "AMB1 requires --wrap=flash_set_status")
ASSERT(flash_reset_status == __wrap_flash_reset_status, "AMB1 requires --wrap=flash_reset_status")
ASSERT(flash_set_extend_addr == __wrap_flash_set_extend_addr, "AMB1 requires --wrap=flash_set_extend_addr")

INCLUDE "export-rom_v02.txt"


MEMORY
{
  TCM (rwx) : ORIGIN = 0x1FFF0000, LENGTH = 65536
  ROM_USED_RAM (rwx)  : ORIGIN = 0x10000bc8, LENGTH = 21560

  BD_RAM (rwx)  : ORIGIN = 0x10006000, LENGTH = 0x3A000
  RECY_RAM (rwx)  : ORIGIN = 0x10002100, LENGTH = 7936

  SDRAM_RAM (rwx)  : ORIGIN = 0x30000000, LENGTH = 2M
}



SECTIONS
{
  /* Both distributed 8195a WLAN archives retain this 8711b call-site name
   * in rltk_stop_softap. Route it to the actual 8195a implementation. */
  PROVIDE(rtl8711b_download_rsvd_page = rtl8195a_download_rsvd_page);
  __rom_bss_start__ = 0x10000300;
  __rom_bss_end__ = 0x10000bc8;

/*
  .ram.start.table :
  {
    __ram_image1_text_start__ = .;
    __ram_start_table_start__ = .;
    KEEP(*(SORT(.start.ram.data*)))
    __ram_start_table_end__ = .;

  } > ROM_USED_RAM
*/

/*  Add . to assign the start address of the section,                                     *
  *  to prevent the change of the start address by ld doing section alignment */


/* these 4 sections is used by ROM global variable */
    /* Don't move them and never add RAM code variable to these sections */
/*
  .ram_image1.text . :
  {
    __image1_validate_code__ = .;
    KEEP(*(.image1.validate.rodata*))
    KEEP(*(.infra.ram.data*))
    KEEP(*(.timer.ram.data*))
    KEEP(*(.cutb.ram.data*))
    KEEP(*(.cutc.ram.data*))
    KEEP(*(.hal.ram.data*))
    __image1_bss_start__ = .;
    __image1_bss_end__ = .;
    __ram_image1_data_end__ = .;

    *(.hal.ram.text*)
    *(.infra.ram.text*)
    __ram_image1_text_end__ = .;
  } > ROM_USED_RAM
*/
/*
  .tcm :
  {
    __tcm_start__ = .;
    *(.tcm.heap)
    *mem.o (.bss)
    *memp.o (.bss)
    __tcm_end__ = .;
  } > TCM
*/

  .bootloader :
  {
    KEEP(*(.loader.data*))
  } > ROM_USED_RAM

  OVERLAY 0x1FFF0000:
  {
   .valid
    {
        __tcm_bss_start__ = .;
        *mem.o (.bss*)
        *memp.o (.bss*)
        *(.tcm.heap)
        __tcm_bss_end__ = .;
    }
    .dummy
    {
        __ram_image1_text_start__ = .;
        __ram_start_table_start__ = .;
        KEEP(*(SORT(.start.ram.data*)))
        __ram_start_table_end__ = .;
		__image1_validate_code__ = .;
        KEEP(*(.image1.validate.rodata*))
        KEEP(*(.infra.ram.data*))
        KEEP(*(.timer.ram.data*))
        KEEP(*(.cutb.ram.data*))
        KEEP(*(.cutc.ram.data*))
        KEEP(*(.hal.ram.data*))
        __image1_bss_start__ = .;
        .ram_image1.bss$$Base = .;
        __image1_bss_end__ = .;
        .ram_image1.bss$$Limit = .;
        __ram_image1_data_end__ = .;

        *(.hal.ram.text*)
        *(.infra.ram.text*)
    }
  } > TCM

  .image2.start.table :
  {
    __ram_image2_text_start__ = .;
    __image2_entry_func__ = .;
    .image2.start.table1$$Base = .;
    KEEP(*(SORT(.image2.ram.data*)))
    __image2_validate_code__ = .;
    KEEP(*(.image2.validate.rodata*))
    KEEP(*(.custom.validate.rodata*))
  } > BD_RAM

  .ram_image2.text :
  {
    KEEP(*(.infra.ram.start*))
    *(.mon.ram.text*)
    *(.hal.flash.text*)
    *(.hal.sdrc.text*)
    *(.hal.gpio.text*)
    *(.fwu.text*)
  } > BD_RAM

  .ram_image2.rodata :
  {
    *(.fwu.rodata*)
  } > BD_RAM

  .ram.data :
  {
    __data_start__ = .;
    . = ALIGN(4);
    __data_end__ = .;
    __ram_image2_text_end__ = .;
  } > BD_RAM

  .ram.bss (NOLOAD) :
  {
    __bss_start__ = .;
    .ram.bss$$Base = .;
    *(.hal.flash.data*)
    *(.hal.sdrc.data*)
    *(.hal.gpio.data*)
    *(.fwu.data*)
    *(.wlan.bdsram.data*)
    *(.bdsram.data*)
    __bss_end__ = .;
    .ram.bss$$Limit = .;
  } > BD_RAM

  .bf_data :
  {
    __buffer_data_start__ = .;
    __buffer_data_end__ = .;

  } > BD_RAM

  .bf_data2 :
  {
    __buffer_data_start2__ = .;
    __buffer_data_end2__ = .;

  } > RECY_RAM

  .sdr_text :
  {
    __sdram_data_start__ = .;
    *(.text*)
    KEEP(*(.init))
    KEEP(*(.fini))
    *(.wlan.text*)
    *(.sdram.text*)
    *(.p2p.text*)
    *(.wps.text*)
    *(.websocket.text*)
    *(.mdns.text*)
  } > SDRAM_RAM

  .sdr_rodata :
  {
    *(.rodata*)
    *(.wlan.rodata*)
    *(.sdram.rodata*)
    *(.p2p.rodata*)
    *(.wps.rodata*)
    *(.websocket.rodata*)
    *(.mdns.rodata*)
    . = ALIGN(4);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN(__preinit_array_end = .);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN(__init_array_end = .);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > SDRAM_RAM

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > SDRAM_RAM

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > SDRAM_RAM

  .sdr_data :
  {
    *(.data*)
    *(.wlan.data*)
    *(.sdram.data*)
    *(.p2p.data*)
    *(.wps.data*)
    *(.websocket.data*)
    *(.mdns.data*)
    . = ALIGN(256);
    *(.fpb.remap*)
    . = ALIGN(4);
    __sdram_data_end__ = .;
  } > SDRAM_RAM

  .sdr_bss (NOLOAD) :
  {
    __sdram_bss_start__ = .;
    *(.bss*)
    *(.wlan.bss*)
    *(.bfsram.data*)
    *(COMMON)
    *(.sdram.bss*)
    *(.p2p.bss*)
    *(.wps.bss*)
    *(.websocket.bss*)
    *(.mdns.bss*)
    __sdram_bss_end__ = .;
  } > SDRAM_RAM

 .heap :
 {
	__end__ = .;
	end = __end__;
	*(.heap*)
	__HeapLimit = .;
 } > BD_RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
	.stack_dummy :
	{
		*(.stack)
	} > BD_RAM

	/* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
	__StackTop = ORIGIN(BD_RAM) + LENGTH(BD_RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

}
