/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

#pragma once

#include <stdint.h>

#define CPU_MODEL(family, chip_id)		(((family >> 24) << 8) | chip_id)
#define CPU_MODEL_ENUM(family, chip_id) (lt_cpu_model_t) CPU_MODEL(family, chip_id)

typedef enum {
	F_RTL8710A = 0x9FFFD543, // Realtek Ameba1
	F_RTL8710B = 0x22E0D6FC, // Realtek AmebaZ (realtek-ambz)
	F_RTL8720C = 0xE08F7564, // Realtek AmebaZ2
	F_RTL8720D = 0x3379CFE2, // Realtek AmebaD
	F_BK7231Q  = 0xAFE81D49, // Beken 7231Q
	F_BK7231T  = 0x675A40B0, // Beken 7231T
	F_BK7231N  = 0x7B3EF230, // Beken 7231N
	F_BK7251   = 0x6A82CC42, // Beken 7251/7252
	F_BL60X	   = 0xDE1270B7, // Boufallo 602
	F_LN882H   = 0xA38090A8, // Lightning LN882H
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
	W302	  = RTL8710BN,
	// Realtek AmebaZ2 (chip_id << 2 | flash_mode)
	RTL8720CM = CPU_MODEL(F_RTL8720C, 0xEC), // 0xFB << 2 | 0
	RTL8720CF = CPU_MODEL(F_RTL8720C, 0xED), // 0xFB << 2 | 1
	RTL8720CX = RTL8720CM,
	// Beken 72XX
	BK7231Q = CPU_MODEL(F_BK7231Q, 0x31), // *SCTRL_CHIP_ID = 0x7231
	BK7231T = CPU_MODEL(F_BK7231T, 0x1A), // *SCTRL_CHIP_ID = 0x7231a
	BK7231N = CPU_MODEL(F_BK7231N, 0x1C), // *SCTRL_CHIP_ID = 0x7231c
	BK7252	= CPU_MODEL(F_BK7251, 0x00),  // TODO
	BL2028N = BK7231N,
	BK7231S = BK7231T,
	BK7231U = BK7231T,
	// Lightning LN882x
	LN882HF = CPU_MODEL(F_LN882H, 0x00), // TODO / QFN24
	LN882HK = CPU_MODEL(F_LN882H, 0x00), // TODO / QFN32
	LN882HC = CPU_MODEL(F_LN882H, 0x00), // TODO / QFN40
} lt_cpu_model_t;
