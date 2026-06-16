// HOST-ONLY shim: a 2 MB RAM-backed flash for unit tests. Not compiled on device.
#include "lt_ota_meta.h"
#include <string.h>

#define HOST_FLASH_LEN 0x200000u
static uint8_t g_flash[HOST_FLASH_LEN];

uint8_t *host_flash_base(void) {
	return g_flash;
}

void host_flash_reset(void) {
	memset(g_flash, 0xFF, sizeof(g_flash));
}

static int h_read(uint32_t off, void *buf, uint32_t len) {
	if (off + len > HOST_FLASH_LEN)
		return -1;
	memcpy(buf, g_flash + off, len);
	return 0;
}

static int h_erase(uint32_t off, uint32_t len) {
	if (off + len > HOST_FLASH_LEN)
		return -1;
	memset(g_flash + off, 0xFF, len);
	return 0;
}

static int h_write(uint32_t off, const void *buf, uint32_t len) {
	if (off + len > HOST_FLASH_LEN)
		return -1;
	// emulate NOR: writes can only clear bits
	const uint8_t *s = (const uint8_t *)buf;
	for (uint32_t i = 0; i < len; i++)
		g_flash[off + i] &= s[i];
	return 0;
}

lt_ota_flash_ops_t host_flash_ops = {h_read, h_erase, h_write};
