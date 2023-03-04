/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

#define CHIP_TYPE(family, chip_id)		(((family >> 24) << 8) | chip_id)
#define CHIP_TYPE_ENUM(family, chip_id) (ChipType) CHIP_TYPE(family, chip_id)

typedef enum {
	// used in UF2 Family ID
	F_RTL8710A = 0x9FFFD543, // Realtek Ameba1
	F_RTL8710B = 0x22E0D6FC, // Realtek AmebaZ (realtek-ambz)
	F_RTL8720C = 0xE08F7564, // Realtek AmebaZ2
	F_RTL8720D = 0x3379CFE2, // Realtek AmebaD
	F_BK7231U  = 0x675A40B0, // Beken 7231U/7231T
	F_BK7231N  = 0x7B3EF230, // Beken 7231N
	F_BK7251   = 0x6A82CC42, // Beken 7251/7252
	F_BL60X	   = 0xDE1270B7, // Boufallo 602
} ChipFamily;

typedef enum {
	// Realtek AmebaZ
	// IDs copied from rtl8710b_efuse.h
	RTL8710BL = CHIP_TYPE(F_RTL8710B, 0xE0), // ???
	RTL8710BN = CHIP_TYPE(F_RTL8710B, 0xFF), // CHIPID_8710BN / QFN32
	RTL8710BU = CHIP_TYPE(F_RTL8710B, 0xFE), // CHIPID_8710BU / QFN48
	RTL8710BX = CHIP_TYPE(F_RTL8710B, 0xF6), // found on an actual RTL8710BX
	RTL8710L0 = CHIP_TYPE(F_RTL8710B, 0xFB), // CHIPID_8710BN_L0 / QFN32
	RTL8711BN = CHIP_TYPE(F_RTL8710B, 0xFD), // CHIPID_8711BN / QFN48
	RTL8711BU = CHIP_TYPE(F_RTL8710B, 0xFC), // CHIPID_8711BG / QFN68
	// Beken 72XX
	BK7231T = CHIP_TYPE(F_BK7231U, 0x1A), // *SCTRL_CHIP_ID = 0x7231a
	BK7231N = CHIP_TYPE(F_BK7231N, 0x1C), // *SCTRL_CHIP_ID = 0x7231c
	BL2028N = CHIP_TYPE(F_BK7231N, 0x1C), // *SCTRL_CHIP_ID = 0x7231c
	BK7252	= CHIP_TYPE(F_BK7251, 0x00),  // TODO
} ChipType;
