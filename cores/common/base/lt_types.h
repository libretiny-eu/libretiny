/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

#include <stdint.h>

#define CPU_MODEL(family, chip_id)		(((family >> 24) << 8) | chip_id)
#define CPU_MODEL_ENUM(family, chip_id) (lt_cpu_model_t) CPU_MODEL(family, chip_id)

#define RESET_REASON_UNKNOWN  REBOOT_REASON_UNKNOWN
#define RESET_REASON_POWER	  REBOOT_REASON_POWER
#define RESET_REASON_BROWNOUT REBOOT_REASON_BROWNOUT
#define RESET_REASON_HARDWARE REBOOT_REASON_HARDWARE
#define RESET_REASON_SOFTWARE REBOOT_REASON_SOFTWARE
#define RESET_REASON_WATCHDOG REBOOT_REASON_WATCHDOG
#define RESET_REASON_CRASH	  REBOOT_REASON_CRASH
#define RESET_REASON_SLEEP	  REBOOT_REASON_SLEEP
#define RESET_REASON_MAX	  REBOOT_REASON_MAX

typedef enum {
	F_RTL8710A = 0x9FFFD543, // Realtek Ameba1
	F_RTL8710B = 0x22E0D6FC, // Realtek AmebaZ (realtek-ambz)
	F_RTL8720C = 0xE08F7564, // Realtek AmebaZ2
	F_RTL8720D = 0x3379CFE2, // Realtek AmebaD
	F_BK7231U  = 0x675A40B0, // Beken 7231U/7231T
	F_BK7231N  = 0x7B3EF230, // Beken 7231N
	F_BK7251   = 0x6A82CC42, // Beken 7251/7252
	F_BL60X	   = 0xDE1270B7, // Boufallo 602
} lt_cpu_family_t;

typedef enum {
	// Realtek AmebaZ
	// IDs copied from rtl8710b_efuse.h
	RTL8710BL = CPU_MODEL(F_RTL8710B, 0xE0), // ???
	RTL8710BN = CPU_MODEL(F_RTL8710B, 0xFF), // CHIPID_8710BN / QFN32
	RTL8710BU = CPU_MODEL(F_RTL8710B, 0xFE), // CHIPID_8710BU / QFN48
	RTL8710BX = CPU_MODEL(F_RTL8710B, 0xF6), // found on an actual RTL8710BX
	RTL8710L0 = CPU_MODEL(F_RTL8710B, 0xFB), // CHIPID_8710BN_L0 / QFN32
	RTL8711BN = CPU_MODEL(F_RTL8710B, 0xFD), // CHIPID_8711BN / QFN48
	RTL8711BU = CPU_MODEL(F_RTL8710B, 0xFC), // CHIPID_8711BG / QFN68
	MX1290	  = RTL8710BN,
	MX1290V2  = RTL8710BX,
	// Realtek AmebaZ2 (chip_id << 2 | flash_mode)
	RTL8720CM = CPU_MODEL(F_RTL8720C, 0xEC), // 0xFB << 2 | 0
	RTL8720CF = CPU_MODEL(F_RTL8720C, 0xED), // 0xFB << 2 | 1
	RTL8720CX = RTL8720CM,
	// Beken 72XX
	BK7231T = CPU_MODEL(F_BK7231U, 0x1A), // *SCTRL_CHIP_ID = 0x7231a
	BK7231N = CPU_MODEL(F_BK7231N, 0x1C), // *SCTRL_CHIP_ID = 0x7231c
	BK7252	= CPU_MODEL(F_BK7251, 0x00),  // TODO
	BL2028N = BK7231N,
	BK7231S = BK7231T,
	BK7231U = BK7231T,
} lt_cpu_model_t;

/**
 * @brief Reset reason enumeration.
 */
typedef enum {
	REBOOT_REASON_UNKNOWN  = 1,
	REBOOT_REASON_POWER	   = 2,
	REBOOT_REASON_BROWNOUT = 3,
	REBOOT_REASON_HARDWARE = 4,
	REBOOT_REASON_SOFTWARE = 5,
	REBOOT_REASON_WATCHDOG = 6,
	REBOOT_REASON_CRASH	   = 7,
	REBOOT_REASON_SLEEP	   = 8,
	REBOOT_REASON_DEBUGGER = 9,
	REBOOT_REASON_MAX	   = 10,
} lt_reboot_reason_t;

/**
 * @brief Flash chip ID structure.
 */
typedef struct {
	uint8_t manufacturer_id;
	uint8_t chip_id;
	uint8_t chip_size_id;
} lt_flash_id_t;

/**
 * @brief Chip's OTA type enumeration.
 */
typedef enum {
	OTA_TYPE_SINGLE = 0,
	OTA_TYPE_DUAL	= 1,
	OTA_TYPE_FILE	= 2,
} lt_ota_type_t;

typedef enum {
	DEBUG_MODE_OFF	= 0,
	DEBUG_MODE_JTAG = 1,
	DEBUG_MODE_SWD	= 2,
} lt_debug_mode_t;
