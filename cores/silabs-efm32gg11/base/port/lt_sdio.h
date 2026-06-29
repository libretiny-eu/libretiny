/* Copyright (c) WGM160P-LibreTiny port 2026-06-10.
 *
 * Minimal PIO-mode driver for the EFM32GG11 SDIO host controller (a standard
 * Arasan SDHCI block), sized for exactly one client: the WF200 on SDIO LOC1.
 * No DMA, no SD-card/eMMC support, no interrupts — bounded polling only.
 *
 * References: SD Host Controller Simplified Spec v3.00 (register semantics),
 * SDIO Simplified Spec v3.00 (CMD5/52/53 formats), Zephyr drivers/sdhc
 * (Apache-2.0, command sequencing cross-check), GSDK bsp_sd_hc.c (MSLA,
 * GG11 magic values only — no code taken).
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	LT_SDIO_OK = 0,
	LT_SDIO_ERR_TIMEOUT, // bounded wait expired (incl. CMD timeout = no card)
	LT_SDIO_ERR_CRC,	 // command or data CRC error
	LT_SDIO_ERR_CMD,	 // other command error (index / end bit)
	LT_SDIO_ERR_DATA,	 // other data error
	LT_SDIO_ERR_ENUM,	 // enumeration handshake failed
} lt_sdio_err_t;

#ifdef __cplusplus
extern "C" {
#endif

// Full bring-up: clocks (USHFRCO 50 MHz), GPIO route LOC1, controller config,
// 400 kHz init clock, CMD0/CMD5/CMD3/CMD7 enumeration, then 25 MHz.
// Leaves the bus in 1-bit mode (the FMAC driver upgrades via CCCR + the
// lt_sdio_enable_high_speed() callback).
lt_sdio_err_t lt_sdio_init(void);
void lt_sdio_deinit(void);

// IO_RW_DIRECT. write=true: sends *byte (and rewrites it with the R5 echo).
// write=false: fills *byte with the read value.
lt_sdio_err_t lt_sdio_cmd52(bool write, uint8_t function, uint32_t address, uint8_t *byte);

// IO_RW_EXTENDED, PIO. length >= 512 must be a multiple of 512 (block mode,
// guaranteed by the FMAC bus layer); length < 512 uses byte mode and the
// caller must pass a length that is a multiple of 4 (the silabs host shim
// sl_wfx_host_sdio_transfer_cmd53 rounds up — the FMAC bus layer itself
// passes exact, unrounded lengths in byte mode).
lt_sdio_err_t lt_sdio_cmd53(bool write, uint8_t function, uint32_t address, uint8_t *buffer, uint16_t length);

// Switch host side to 4-bit + high-speed clock. Called by the host port from
// sl_wfx_host_sdio_enable_high_speed_mode() AFTER the driver has set the
// matching CCCR bits on the card.
lt_sdio_err_t lt_sdio_enable_high_speed(void);

uint32_t lt_sdio_ocr(void); // CMD5 OCR (debug/probe)
uint32_t lt_sdio_rca(void); // assigned RCA (debug/probe)

#ifdef __cplusplus
}
#endif
