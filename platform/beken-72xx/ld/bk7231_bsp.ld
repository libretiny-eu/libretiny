/*
 * Script for GNU linker.
 * Describes layout of sections, location of stack.
 *
 * In this case vectors are at location 0 (reset @ 0x08)
 *
 * +------------+  0x00400020
 * data         |
 * end
 * |(heap)      |
 * .            .
 * .            .
 * |(heap limit)|
 *
 * |- - - - - - |
 * stack bottom    256k
 * +------------+
 *
 * +------------+  0x0000000
 * |vectors     |
 * |            |
 * |------------+
 * |text        |
 * |data        |
 * |            |  1024k
 * +------------+
 */

/* Split memory into area for vectors and ram */
MEMORY
{
	flash  (rx) : ORIGIN = 0x00010000, LENGTH = 1912K
	ram   (rw!x): ORIGIN = 0x00400100, LENGTH = 256k - 0x100
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vector_start);
_vector_start = ORIGIN(flash);

SECTIONS
{
	/* vectors go to vectors region */
	. = ORIGIN(flash);
	.vectors :
	{
		KEEP(*(*.vectors))
		KEEP( *(*.boot))
	} > flash

	/* instructions go to the text region*/

	. = ALIGN(0x8);
	/* code, instructions.for example: i=i+1; */
	.text :
	{
		*(.text)
		*(.text.*)
		*(.stub)

		/* https://www.embedded.com/building-bare-metal-arm-systems-with-gnu-part-3/ */
		KEEP(*crtbegin.o(.ctors))
		KEEP(*(EXCLUDE_FILE (*ctrend.o) .ctors))
		KEEP(*(SORT(.ctors.*)))
		KEEP(*crtend.o(.ctors))
		KEEP(*crtbegin.o(.dtors))
		KEEP(*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP(*(SORT(.dtors.*)))
		KEEP(*crtend.o(.dtors))

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

		. = ALIGN (4);
		__cmd_table_start__ = .;
		KEEP(*(.cmd.table.data*))
		__cmd_table_end__ = .;

		/* https://community.silabs.com/s/article/understand-the-gnu-linker-script-of-cortex-m4?language=en_US */
		KEEP(*(.init))
		KEEP(*(.fini))
		*(.init)
		*(.fini)

		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7t) *(.glue_7)
	} > flash

	/* read only data.for example: const int rom_data[3]={1,2,3}; */
	.rodata ALIGN(8) :
	{
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
	} > flash

	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx*)
		*(.gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > flash

	/* globals.for example: int ram_data[3]={4,5,6}; */
	/* VMA in RAM, but keep LMA in flash */
	_begin_data = .;
	.data : AT ( _begin_data )
	{
		*(.data .data.*)
		*(.sdata)
		*(.gnu.linkonce.d*)
		SORT(CONSTRUCTORS)
	} >ram

	/* Loader will copy data from _flash_begin to _ram_begin..ram_end */
	_data_flash_begin = LOADADDR(.data);
	_data_ram_begin = ADDR(.data);
	_data_ram_end = .;

	/* uninitialized data section - global   int i; */
	.bss ALIGN(8):
	{
		_bss_start = .;
		*boot_handlers.O(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*(.bss .bss.*)
		*(.scommon)
		*(.sbss)
		*(.dynbss)
		*(COMMON)
		/* Align here to ensure that the .bss section occupies space up to
		_end.  Align after .bss to ensure correct alignment even if the
		.bss section disappears because there are no input sections.  */
		. = ALIGN(32 / 8);
		_bss_end = .;
	} > ram						/* in RAM */

	. = ALIGN (8);
	_empty_ram = .;

	/* This symbol defines end of code/data sections. Heap starts here. */
	PROVIDE(end = .);
}

GROUP(
	libgcc.a
	libg.a
	libc.a
	libm.a
	libnosys.a
)
