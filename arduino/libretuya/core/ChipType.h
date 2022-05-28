/* Copyright (c) Kuba Szczodrzyński 2022-05-28. */

enum ChipFamily {
	// copied from UF2 families (first nibble)
	RTL8710A = 0x9F, // Realtek Ameba1
	RTL8710B = 0x22, // Realtek AmebaZ (realtek-ambz)
	RTL8720C = 0xE0, // Realtek AmebaZ2
	RTL8720D = 0x33, // Realtek AmebaD
	BK7231T	 = 0x67, // Beken 7231T
	BK7231N	 = 0x7B, // Beken 7231N
	BL602	 = 0xDE, // Boufallo 602
	XR809	 = 0x51, // Xradiotech 809
};

enum ChipType {
	// copied from rtl8710b_efuse.h
	RTL8710BL = (RTL8710B << 8) | 0xE0, // ???
	RTL8710BN = (RTL8710B << 8) | 0xFF, // CHIPID_8710BN / QFN32
	RTL8710BU = (RTL8710B << 8) | 0xFE, // CHIPID_8710BU / QFN48
	RTL8710BX = (RTL8710B << 8) | 0xFB, // CHIPID_8710BN_L0 / QFN32
	RTL8711BN = (RTL8710B << 8) | 0xFD, // CHIPID_8711BN / QFN48
	RTL8711BU = (RTL8710B << 8) | 0xFC, // CHIPID_8711BG / QFN68
};
