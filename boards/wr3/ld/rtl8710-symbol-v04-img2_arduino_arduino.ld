

ENTRY(Reset_Handler)

INCLUDE "export-rom_v04.txt"


MEMORY
{
  ROM_USED_RAM (rwx): ORIGIN = 0x10000bc8, LENGTH = 21560	/* end 0x10006000 */
  ROM_HEAP (rwx)	: ORIGIN = 0x10002400, LENGTH = 3K		/* end 0x10003000 */
  RAM_HEAP1 (rwx)	: ORIGIN = 0x10003000, LENGTH = 12K		/* end 0x10006000 */
  BD_RAM (rwx)		: ORIGIN = 0x10006000, LENGTH = 424K	/* end 0x10070000 */
  TCM (rwx)			: ORIGIN = 0x1FFF0000, LENGTH = 64K		/* end 0x20000000 */
  TCM_TAB (rwx)		: ORIGIN = 0x1FFFFD00, LENGTH = 768		/* end 0x20000000 */
  SDRAM_RAM (rwx)	: ORIGIN = 0x30000000, LENGTH = 2M 		/* end 0x30200000 */
}

SECTIONS
{
  __rom_bss_start__ = 0x10000300;
  __rom_bss_end__ = 0x10000bc8;

  .bootloader :
  {
    KEEP(*(.loader.data*))
  } > ROM_USED_RAM

  .romheap :
  {
	__rom_heap_start__ = .; 
	end = __rom_heap_start__;
  	. = ALIGN(0x1000);
  } > ROM_HEAP

  .ram_heap1 : 
  {
	__rom_heap_end__ = .; 
  	__ram_heap1_start__ = .;
  /*   *(.heap1*) */
  } > RAM_HEAP1

  .ram.start.table :
  {
    __ram_image1_text_start__ = .;
    __ram_start_table_start__ = .;
    KEEP(*(SORT(.start.ram.data*)))
    __ram_start_table_end__ = .;
    
  } > ROM_USED_RAM


/*  Add . to assign the start address of the section,
  *  to prevent the change of the start address by ld doing section alignment */  
  .ram_image1.text . :
  {
  	/* these 4 sections is used by ROM global variable */
  	/* Don't move them and never add RAM code variable to these sections */
    __image1_validate_code__ = .;
    KEEP(*(.image1.validate.rodata*))
    KEEP(*(.infra.ram.data*))
    KEEP(*(.timer.ram.data*))
    KEEP(*(.cutb.ram.data*))
    KEEP(*(.cutc.ram.data*))
    KEEP(*(.hal.ram.data*))
    .ram_image1.bss$$Base = .;
    __image1_bss_start__ = .;
    .ram_image1.bss$$Limit = .;
    __image1_bss_end__ = .;
    __ram_image1_data_end__ = .;
    
    *(.hal.ram.text*)    
    *(.infra.ram.text*)
    __ram_image1_text_end__ = .;
  } > ROM_USED_RAM

  .tcm :
  {
	__tcm_heap_start__ = .;
    *(.tcm.heap)
  } > TCM

  .soc_ps_monitor :
  { 
  	__tcm_heap_end__ = .;
  } > TCM_TAB

  .image2.start.table :
  {
  	__ram_heap1_end__ = .;
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

    PROVIDE (_init = .);

    . = ALIGN(4);
    KEEP(*(.init))

    /* init data */
    . = ALIGN(4);
    PROVIDE (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE (__init_array_end = .);

    . = ALIGN(4);
    KEEP(*(.fini))

    . = ALIGN(4);
    PROVIDE (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE (__fini_array_end = .);

    *(.mon.ram.text*)
    *(.rodata*)
    *(.hal.flash.text*)    
    *(.hal.sdrc.text*)
    *(.hal.gpio.text*)
    *(.fwu.rodata*)
    *(.fwu.text*)
    *(.otg.rom.text*)
    *(.text*)
    *(.sdram.text*)
    *(.p2p.text*)
    *(.wps.text*)
    *(.websocket.text*)
    *(.sdram.rodata*)
    *(.p2p.rodata*)
    *(.wps.rodata*)
    *(.websocket.rodata*)
    . = ALIGN(4);
    xHeapRegions = .;
    LONG(__ram_heap1_start__)
    LONG(__ram_heap1_end__ - __ram_heap1_start__)   
    LONG(__ram_heap2_start__) 
    LONG(__ram_heap2_end__ - __ram_heap2_start__)
    LONG(__sdram_heap_start__)
    LONG(__sdram_heap_end__ - __sdram_heap_start__) 
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)

  } > BD_RAM

  .ram.data : 
  {
    __data_start__ = .;
    *(.data*)
    *(.sdram.data*)
    *(.p2p.data*)
    *(.wps.data*)
    *(.websocket.data*)
    __data_end__ = .;
 	. = ALIGN(4);
   	__ram_image2_text_end__ = .;    
  } > BD_RAM

  .ram.bss :
  {
    __bss_start__ = .;
    .ram.bss$$Base = .;
    *(.hal.flash.data*)    
    *(.hal.sdrc.data*)    
    *(.hal.gpio.data*)
    *(.fwu.data*)
    *(.bss*)
    *(COMMON)
    *(.sdram.bss*)
    *(.p2p.bss*)
    *(.wps.bss*)
    *(.websocket.bss*)
    *(.sdram.ssl_ram_map*)
    *(.ssl_ram_map*)
    *mem.o (.bss) 
    *memp.o (.bss)  
    *(.bdsram.data*)
    *(.bfsram.data*)
    __bss_end__ = .;
    .ram.bss$$Limit = .;
  } > BD_RAM

  .bf_data : /* test */
  {
    __buffer_data_start__ = .;
    *(.sdram*)
    *(.data*)
    __buffer_data_end__ = .;
    
  } > BD_RAM

  .ram_heap2 : 
  {
    . = ALIGN(8);
  	__ram_heap2_start__ = .;
    KEEP(*(.sram.heap*)) /* ucHeap */
  } > BD_RAM
  __ram_heap2_end__ = 0x10070000;

  .sdr_text :
  {
    __sdram_data_start__ = .;
  } > SDRAM_RAM

  .sdr_rodata :
  {
  } > SDRAM_RAM

  .sdr_data : 
  {
    __sdram_data_end__ = .;
  } > SDRAM_RAM

  .sdr_bss :
  {
    __sdram_bss_start__ = .;
	*(.uvc.ram.bss)
    __sdram_bss_end__ = .;
    . = ALIGN(8);
  	__sdram_heap_start__ = .;
  } > SDRAM_RAM
  __sdram_heap_end__ = 0x30200000;

	.boot.head :
	{
		KEEP(*(.loader.head*))
	}
	
}


