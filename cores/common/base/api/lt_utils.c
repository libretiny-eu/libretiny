/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_utils.h"

void lt_rand_bytes(uint8_t *buf, size_t len) {
	int *data = (int *)buf;
	size_t i;
	for (i = 0; len >= sizeof(int); len -= sizeof(int)) {
		data[i++] = rand();
	}
	if (len) {
		int rem				= rand();
		unsigned char *pRem = (unsigned char *)&rem;
		memcpy(buf + i * sizeof(int), pRem, len);
	}
}

void hexdump(const uint8_t *buf, size_t len, uint32_t offset, uint8_t width) {
	uint16_t pos = 0;
	while (pos < len) {
		// print hex offset
		printf("%06lx ", offset + pos);
		// calculate current line width
		uint8_t lineWidth = MIN(width, len - pos);
		// print hexadecimal representation
		for (uint8_t i = 0; i < lineWidth; i++) {
			if (i % 8 == 0) {
				printf(" ");
			}
			printf("%02x ", buf[pos + i]);
		}
		// print ascii representation
		printf(" |");
		for (uint8_t i = 0; i < lineWidth; i++) {
			char c = buf[pos + i];
			putchar((c >= 0x20 && c <= 0x7f) ? c : '.');
		}
		puts("|\r");
		pos += lineWidth;
	}
}

char *lt_btox(const uint8_t *src, int len, char *dest) {
	// https://stackoverflow.com/a/53966346
	const char hex[] = "0123456789abcdef";
	len *= 2;
	dest[len] = '\0';
	while (--len >= 0)
		dest[len] = hex[(src[len >> 1] >> ((1 - (len & 1)) << 2)) & 0xF];
	return dest;
}

uint8_t *lt_xtob(const char *src, int len, uint8_t *dest) {
	// https://gist.github.com/vi/dd3b5569af8a26b97c8e20ae06e804cb

	// mapping of ASCII characters to hex values
	// (16-byte swapped to reduce XOR 0x10 operation)
	const uint8_t mapping[] = {
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
		0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
	};

	int j = 0;
	uint8_t idx0;
	uint8_t idx1;
	for (int i = 0; i < len; i += 2) {
		idx0	  = ((uint8_t)src[i + 0] & 0x1F);
		idx1	  = ((uint8_t)src[i + 1] & 0x1F);
		dest[j++] = (mapping[idx0] << 4) | (mapping[idx1] << 0);
	}
	return dest;
}
