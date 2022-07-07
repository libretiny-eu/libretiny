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
	tcm   (rw!x): ORIGIN = 0x003F0000, LENGTH = 60k - 512
	itcm   (rwx): ORIGIN = 0x003FEE00, LENGTH = 4k + 512
	ram   (rw!x): ORIGIN = 0x00400100, LENGTH = 192k - 0x100
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

	. = ORIGIN(itcm);
	.itcm.code ALIGN(8) :
	{
		/* itcm 4KB code */
		*(.text.intc_hdl_entry)
		*(.text.intc_irq)
		*(.text.intc_fiq)
		*(.text.bk_timer_isr)
		*(.text.power_save_wakeup_isr)
		*(.text.bmsg_rx_sender)
		*(.text.bmsg_null_sender)
		*(.text.fclk_get_tick)
		*(.text.flash_read_sr)
		*(.text.flash_write_sr)
		*(.text.flash_clr_qwfr)
		*(.text.set_flash_protect)
		*(.text.flash_read)
		*(.text.flash_read_data)
		*(.text.flash_set_qe)
		*(.text.flash_set_qwfr)
		*(.text.flash_set_line_mode*)
		*(.text.flash_get_line_mode)
		*(.text.flash_write)
		*(.text.flash_ctrl)
		*(.text.power_save_dtim_wake)
		*(.text.sctrl_fix_dpll_div)

		*(.text.vTaskSuspendAll)
		*(.text.xTaskGetTickCount)
		*(.text.xTaskGetTickCountFromISR)
		*(.text.vTaskStepTick)
		*(.text.xTaskIncrementTick)
		*(.text.xTaskResumeAll)
		*(.text.vTaskSwitchContext)
		*(.text.vApplicationIdleHook)
		*(.text.platform_is_in_irq_context)
		*(.text.platform_is_in_fiq_context)
		*(.text.platform_is_in_interrupt_context)
		*(.text.portENABLE_IRQ)
		*(.text.portENABLE_FIQ)
		*(.text.portDISABLE_FIQ)
		*(.text.portDISABLE_IRQ)
		*(.text.vPortEnterCritical)
		*(.text.vPortExitCritical)
	} > itcm AT>flash
	_itcmcode_flash_begin = LOADADDR(.itcm.code);
	_itcmcode_ram_begin = ADDR(.itcm.code);
	_itcmcode_ram_end = _itcmcode_ram_begin + SIZEOF(.itcm.code);

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

	. = ORIGIN(tcm);
	.tcm ALIGN(8) :
	{
		*apm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*apm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_ble_init.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_ble_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_ble_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_ble.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_comm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_sdp.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_sec.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*app.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*arbitrate.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*arch_main*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ate_app*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*bam_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*bam.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*bk_timer.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*bk7011_cal*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*bk7231N_cal.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*BkDriverFlash.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_aes.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_main.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_rf_port.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_rf_xvr.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_ui.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble_util_buf.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ble.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*chan.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*cmd_evm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*cmd_rx_sensitivity.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*comm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*comm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*common_list.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*common_utils.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*common.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ctrl_iface.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dbg_mwsgen.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dbg_swdiag.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dbg_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dbg.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dd.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dhcp-server-main.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dhcp-server.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dhcp.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dma.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*dns.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*drv_model.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ecc_p256.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*eloop.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*etharp.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*fake_clock.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*flash.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gapc_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gapc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gapm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gapm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gattc_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gattc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gattm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gattm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*gpio.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*h4tl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hal_dma.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hal_machw.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hci_fc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hci_tl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hci.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*heap_4.marm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*hostapd*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ieee802_11_demo.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*igmp.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*intc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ip4_addr.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ip4.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*irda*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ke_env.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*kernel_event.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*kernel_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*kernel.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*l2cc_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*l2cc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*l2cm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*llc_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*llc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld_adv.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld_con.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld_init.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld_per_adv.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld_test.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*lld.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*llm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*llm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mac_phy_bypass.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*main_none.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*main_supplicant.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*manual_cal_bk7231U.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*me_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*me.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mem_arch.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mm_bcn.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mm_timer.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*mm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*net.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*netif.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*param_config.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*phy_trident*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ping.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*power_save*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*prf_utils.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*prf.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*ps.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*pwm_bk7231n.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*raw.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rf_xvr.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*role_launch.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*RomCallFlash.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rtos_pub*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rw_ieee80211.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rw_msg_rx.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwble.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwip_driver.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwip.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwip.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwnx_intf*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rwnx.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rx_sensitivity.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rx_swdesc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rxl_cntrl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rxl_hwdesc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*rxu_cntrl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sa_ap.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sa_station.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*saradc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*scan_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*scan.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*scanu_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*scanu.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sch_alarm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sch_arb.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sch_plan.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sch_prog.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sch_slice.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sdp_common.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sdp_service_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sdp_service.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sm_task.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sockets.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*spi*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sta_mgmt.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*start_type.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sys_arch.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*sys_ctrl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tasks.marm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tcp_in.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tcp.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tcpip.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*td.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*temp_detect.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*timeouts.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*timers.marm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tx_evm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*tx_swdesc.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*txl_buffer.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*txl_cfm.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*txl_cntrl.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*uart_ble.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*uart.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*vif_mgmt.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wdt.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wlan_cli*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wlan_ui*.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wpa_debug.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wpa_psk_cache.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wpa_psk_cache.o(.bss .bss.* .scommon .sbss .dynbss COMMON)
		*wpa_supplicant.o(.bss .bss.* .scommon .sbss .dynbss COMMON)

		/* *memp.o(.bss .bss.* .scommon .sbss .dynbss COMMON) */
		/* *mem.o(.bss .bss.* .scommon .sbss .dynbss COMMON) */
	} >tcm AT>flash
	_tcmbss_start = ADDR(.tcm);
	_tcmbss_end = _tcmbss_start + SIZEOF(.tcm);

	. = ORIGIN(ram);
	/* globals.for example: int ram_data[3]={4,5,6}; */
	/* VMA in RAM, but keep LMA in flash */
	_begin_data = .;
	.data :
	{
		*(.data .data.*)
		*(.sdata)
		*(.gnu.linkonce.d*)
		SORT(CONSTRUCTORS)
	} >ram AT>flash
	_end_data = .;

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
