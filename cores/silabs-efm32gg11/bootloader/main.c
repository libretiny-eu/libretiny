// Copyright (c) WGM160P-LibreTiny port 2026-06-12.
// SPDX-License-Identifier: MIT
//
// MIT first-stage bootloader for the silabs-efm32gg11 family (EFM32GG11B820,
// WGM160P). Lives at flash 0x0. At reset it reads the power-fail-safe ping-pong
// OTA metadata, validates the selected app bank's image (CRC32), handles
// trial/rollback accounting, then jumps to the bank. The GSDK startup runs
// .data/.bss init + SystemInit and then `bl __START`; building with -D__START=main
// lands Reset_Handler directly here (plain C, no newlib _start / libc_init_array).

#include "em_device.h"
#include "em_msc.h"
#include "lt_ota_meta.h"
#include <stdint.h>
#include <string.h>

// Standalone MSC-backed flash ops for the metadata page (read via memory map,
// erase/write via emlib MSC) so the bootloader can decrement the trial counter.
static int btl_read(uint32_t off, void *buf, uint32_t len) {
	memcpy(buf, (const void *)(uintptr_t)off, len);
	return 0;
}

static int btl_erase(uint32_t off, uint32_t len) {
	for (uint32_t a = off; a < off + len; a += 4096u)
		if (MSC_ErasePage((uint32_t *)(uintptr_t)a) != mscReturnOk)
			return -1;
	return 0;
}

static int btl_write(uint32_t off, const void *buf, uint32_t len) {
	if ((off & 0x3u) || (len & 0x3u))
		return -1;
	if (MSC_WriteWord((uint32_t *)(uintptr_t)off, buf, len) != mscReturnOk)
		return -1;
	return 0;
}

lt_ota_flash_ops_t btl_flash_ops = {btl_read, btl_erase, btl_write};

static void jump_to_bank(uint8_t bank) {
	uint32_t base = lt_ota_bank_offset(bank);
	uint32_t msp  = *(volatile uint32_t *)(base + 0);
	uint32_t pc	  = *(volatile uint32_t *)(base + 4);
	SCB->VTOR	  = base; // app's SystemInit re-sets this; belt+braces
	__asm volatile("msr msp, %0" ::"r"(msp));
	((void (*)(void))pc)();
}

static int vectortable_sane(uint8_t bank) {
	uint32_t base = lt_ota_bank_offset(bank);
	uint32_t msp  = *(volatile uint32_t *)(base + 0);
	uint32_t pc	  = *(volatile uint32_t *)(base + 4);
	if (msp < 0x20000000u || msp > 0x20080000u)
		return 0;
	if (pc < base || pc >= base + LT_OTA_BANK_LEN)
		return 0;
	if ((pc & 1u) == 0)
		return 0;
	return 1;
}

int main(void) {
	MSC_Init();
	lt_ota_meta_t m;
	uint8_t boot;
	if (lt_ota_meta_load(&btl_flash_ops, &m)) {
		boot = m.boot_bank;
		if (m.bank[boot].state == LT_OTA_BANK_TRIAL) {
			if (m.attempts == 0) {
				uint8_t other = boot ^ 1u;
				if (lt_ota_meta_bank_valid(&btl_flash_ops, &m, other)) {
					m.boot_bank = other;
					boot		= other;
					lt_ota_meta_store(&btl_flash_ops, &m);
				}
			} else {
				m.attempts--;
				lt_ota_meta_store(&btl_flash_ops, &m);
			}
		}
		if (lt_ota_meta_bank_valid(&btl_flash_ops, &m, boot))
			jump_to_bank(boot);
		if (lt_ota_meta_bank_valid(&btl_flash_ops, &m, boot ^ 1u))
			jump_to_bank(boot ^ 1u);
	}
	// No/invalid metadata (first bench flash): boot bank A if it looks sane.
	if (vectortable_sane(0))
		jump_to_bank(0);
	if (vectortable_sane(1))
		jump_to_bank(1);
	while (1) {} // nothing bootable -> park for SWD recovery
}
