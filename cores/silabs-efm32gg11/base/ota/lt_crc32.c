#include "lt_crc32.h"

uint32_t lt_crc32(uint32_t crc, const void *data, size_t len) {
	const uint8_t *p = (const uint8_t *)data;
	while (len--) {
		crc ^= *p++;
		for (int k = 0; k < 8; k++)
			crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
	}
	return crc;
}
