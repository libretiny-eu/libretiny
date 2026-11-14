/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */
/* Ported to Realtek Ameba1 (RTL8195A / RTL8711AM). */

#include <libretiny.h>
#include <sdk_private.h>

#include <hal_efuse.h>

/* Boot-crash instrumentation, see fixups/boot_trace.c (temporary). */
extern void lt_amb1_trace(const char *tag);

static bool read_factory_mac(uint8_t *mac) {
	/* WLAN logical map is packet-encoded, not the 32-byte user OTP area.
	 * RTL8195A _ReadEfuseInfo8195a copies map[0x11A..0x11F]. */
	uint8_t map[512];
	memset(map, 0xFF, sizeof(map));
	uint16_t pos	   = 0;
	unsigned mac_words = 0;
	uint32_t ctrl	   = HAL_READ32(SYSTEM_CTRL_BASE, REG_SYS_EFUSE_CTRL);
	while (pos < 256) {
		uint8_t header, ext;
		if (!HALEFUSEOneByteReadRAM(ctrl, pos++, &header, L25EOUTVOLTAGE))
			return false;
		if (header == 0xFF)
			break;
		unsigned section = header >> 4;
		unsigned mask	 = header & 15;
		if ((header & 31) == 15) {
			if (pos >= 256 || !HALEFUSEOneByteReadRAM(ctrl, pos++, &ext, L25EOUTVOLTAGE))
				return false;
			section = (header >> 5) | ((ext & 0xF0) >> 1);
			mask	= ext & 15;
		}
		for (unsigned word = 0; word < 4; word++) {
			if (mask & (1 << word))
				continue;
			for (unsigned byte = 0; byte < 2; byte++) {
				uint8_t value;
				if (pos >= 256 || !HALEFUSEOneByteReadRAM(ctrl, pos++, &value, L25EOUTVOLTAGE))
					return false;
				if (section < 64)
					map[section * 8 + word * 2 + byte] = value;
			}
			unsigned logical = section * 8 + word * 2;
			if (logical >= 0x11A && logical <= 0x11E)
				mac_words |= 1u << ((logical - 0x11A) / 2);
		}
	}
	memcpy(mac, map + 0x11A, 6);
	uint8_t any = 0;
	for (unsigned i = 0; i < 6; i++)
		any |= mac[i];
	return mac_words == 7 && any && !(mac[0] & 1);
}

void lt_get_device_mac(uint8_t *mac) {
	static bool trace_once;
	if (!trace_once) {
		trace_once = true;
		lt_amb1_trace("device -> lt_get_device_mac (efuse)");
	}
	static uint8_t factory_mac[6];
	static bool valid;
	if (!valid)
		valid = read_factory_mac(factory_mac);
	if (valid) {
		memcpy(mac, factory_mac, 6);
	} else {
		memset(mac, 0, 6);
		LT_E("Factory WLAN MAC unavailable (efuse)");
	}
}

bool lt_reboot_download_mode() {
	// 8195a has no UART download ROM (flash via SPI/JTAG). AmebaZ poked a
	// NORESET_FF register here; on 8195a just reset.
	sys_reset();
	while (1) {}
	return true;
}

bool lt_set_debug_mode(lt_debug_mode_t mode) {
	switch (mode) {
		case DEBUG_MODE_OFF:
			sys_jtag_off();
			return true;
		case DEBUG_MODE_SWD:
			// TODO(amb1): enable the SWD/JTAG pinmux on 8195a during bring-up.
			return true;
		default:
			return false;
	}
}
