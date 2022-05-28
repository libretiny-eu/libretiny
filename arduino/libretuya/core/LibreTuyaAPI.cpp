/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "LibreTuyaAPI.h"

String ipToString(const IPAddress &ip) {
	char szRet[16];
	sprintf(szRet, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
	return String(szRet);
}

/**
 * @brief Generate random bytes using rand().
 *
 * @param buf destination pointer
 * @param len how many bytes to generate
 */
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

/**
 * @brief Print data pointed to by buf in hexdump-like format (hex+ASCII).
 *
 * @param buf source pointer
 * @param len how many bytes to print
 * @param offset increment printed offset by this value
 * @param width how many bytes on a line
 */
void hexdump(uint8_t *buf, size_t len, uint32_t offset, uint8_t width) {
	uint16_t pos = 0;
	while (pos < len) {
		// print hex offset
		printf("%06x ", offset + pos);
		// calculate current line width
		uint8_t lineWidth = min(width, len - pos);
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
			printf("%c", isprint(c) ? c : '.');
		}
		printf("|\n");
		pos += lineWidth;
	}
}

/**
 * @brief Get LibreTuya version string.
 */
const char *LibreTuya::getVersion() {
	return LT_VERSION_STR;
}

/**
 * @brief Get board name.
 */
const char *LibreTuya::getBoard() {
	return LT_BOARD_STR;
}

static char *deviceName = NULL;

/**
 * @brief Get device friendly name in format "LT-<board>-<chip id>".
 * Can be used as hostname.
 */
const char *LibreTuya::getDeviceName() {
	if (deviceName)
		return deviceName;
	uint32_t chipId = getChipId();
	uint8_t *id		= (uint8_t *)&chipId;

	const char *board = getBoard();
	uint8_t boardLen  = strlen(board);
	deviceName		  = (char *)malloc(3 + boardLen + 1 + 6 + 1);

	sprintf(deviceName, "LT-%s-%02x%02x%02x", board, id[0], id[1], id[2]);
	return deviceName;
}
