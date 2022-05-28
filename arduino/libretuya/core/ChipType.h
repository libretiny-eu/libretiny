/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

enum ChipFamily {
	// used in UF2 Family ID
	RTL8710A = 0x9FFFD543, // Realtek Ameba1
	RTL8710B = 0x22E0D6FC, // Realtek AmebaZ (realtek-ambz)
	RTL8720C = 0xE08F7564, // Realtek AmebaZ2
	RTL8720D = 0x3379CFE2, // Realtek AmebaD
	BK7231T	 = 0x675A40B0, // Beken 7231T
	BK7231N	 = 0x7B3EF230, // Beken 7231N
	BL602	 = 0xDE1270B7, // Boufallo 602
	XR809	 = 0x51E903A8, // Xradiotech 809
};

enum ChipType {
	// IDs copied from rtl8710b_efuse.h
	RTL8710BL = ((RTL8710B >> 24) << 8) | 0xE0, // ???
	RTL8710BN = ((RTL8710B >> 24) << 8) | 0xFF, // CHIPID_8710BN / QFN32
	RTL8710BU = ((RTL8710B >> 24) << 8) | 0xFE, // CHIPID_8710BU / QFN48
	RTL8710BX = ((RTL8710B >> 24) << 8) | 0xFB, // CHIPID_8710BN_L0 / QFN32
	RTL8711BN = ((RTL8710B >> 24) << 8) | 0xFD, // CHIPID_8711BN / QFN48
	RTL8711BU = ((RTL8710B >> 24) << 8) | 0xFC, // CHIPID_8711BG / QFN68
};
