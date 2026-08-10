ENTRY(Reset_Handler)

INCLUDE "rlx8721d_layout_is.ld"

SECTIONS
{
	.rom.text : { } > IROM_NS
	.rom.rodata : { } > DROM_NS
	.hal.rom.bss : { } > ROMBSS_RAM_COM
	.hal.ns_rom.bss : { } > ROMBSS_RAM_NS

	/* image2: normal image start */
	.ram_image2.entry :
	{
		__ram_image2_text_start__ = .;
		__image2_entry_func__ = .;
		KEEP(*(SORT(.image2.entry.data*)))

		__image2_validate_code__ = .;
		KEEP(*(.image2.validate.rodata*)) 

	} > BD_RAM_NS

	.ram_image2.text :
	{
		__ram_text_start__ = .;
		*(.image2.ram.text*)
		*(.image2.net.ram.text*)
		__ram_text_end__ = .;
	} > BD_RAM_NS

	.ram_image2.data : 
	{
		__data_start__ = .;
		*(.data*)
		*(.image2.ram.data*)
		*(.image2.net.ram.data*)
		__data_end__ = .;
		__ram_image2_text_end__ = .;
		. = ALIGN(16);	
	} > BD_RAM_NS
  
	.ram_image2.bss :
	{
		__bss_start__ = .;
		*(.bss*)
		*(.image2.ram.bss*)
		*(.image2.net.ram.bss*)
		*(COMMON)
		__bss_end__ = .;
	} > BD_RAM_NS

	.ram_image2.nocache.data :
	{
		. = ALIGN (32);
		__ram_nocache_start__ = .;
		*(.bdsram.data*)
		*(.image2.ram.bdsram.data*)
		*(.image2.net.ram.bdsram.data*)
		. = ALIGN (32);
		__ram_nocache_end__ = .;
	} > BD_RAM_NS

	.ram_heap.data : 
	{
		. = ALIGN (32);
		*(.bfsram.data*)
		*(.image2.ram.bfsram.data*)
		*(.image2.net.ram.bfsram.data*)
		end = .;
		*(.heap.stdlib*)
		. = . + 4096;
		__bfsram_end__ = .;
	} > BD_RAM_NS

	.xip_image2.text : 
	{
		__flash_text_start__ = .;

		*(.img2_custom_signature*)

		*(.text*)
	/*	*(.image2.net.ram.text*) */		
		*(.rodata*)
		*(.image2.ram.rodata*)
		*(.image2.net.ram.rodata*)

		/* Add This for C++ support */
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
		. = ALIGN(4);
		__init_array_start = .;
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		__init_array_end = .;
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		__fini_array_end = .;	
		/*-----------------*/		

		. = ALIGN (4);
		__cmd_table_start__ = .;
		KEEP(*(.cmd.table.data*))
		__cmd_table_end__ = .;

		__flash_text_end__ = .;
	
		. = ALIGN (16);
	} > KM4_IMG2

	/* Add This for C++ support */
	.ARM.extab  : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*) 

	} > KM4_IMG2

	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > KM4_IMG2
	/*-----------------*/

	.bluetooth_trace.text :
	{
		__btrace_start__ = .;
		*(.BTTRACE)
		__btrace_end__ = .;
	} > BTRACE

	/* PSRAM_NS image start */
	.psram_image2.text :
	{
		__psram_image2_text_start__ = .;
		*(.psram.text*)
		__psram_image2_text_end__ = .;
	} > PSRAM_NS

	.psram_image2.data : 
	{
		. = ALIGN (32);
		*(.psram.data*)
		. = ALIGN (32);
		*(.psram.rodata*)
	} > PSRAM_NS
  
	.psram_image2.bss :
	{
		. = ALIGN (32);
		__psram_bss_start__ = .;
		*(.psram.bss*)
		__psram_bss_end__ = .;
	} > PSRAM_NS

	.psram_heap.data : 
	{
		. = ALIGN (32);
		*(.psram.heap*)
		*(.image2.net.ram.psram.heap*)
	} > PSRAM_NS
}

__sram_end__ = ORIGIN(BD_RAM_NS) + LENGTH(BD_RAM_NS);
