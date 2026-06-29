#include "lt_crc32.h"
#include <assert.h>
#include <string.h>

int main(void) {
	// Known IEEE CRC-32 of "123456789" is 0xCBF43926.
	const char *s = "123456789";
	uint32_t c	  = lt_crc32(0xFFFFFFFFu, s, 9) ^ 0xFFFFFFFFu;
	assert(c == 0xCBF43926u);

	// Incremental == one-shot.
	uint32_t a = lt_crc32(0xFFFFFFFFu, s, 4);
	a		   = lt_crc32(a, s + 4, 5);
	assert((a ^ 0xFFFFFFFFu) == 0xCBF43926u);

	// Empty span leaves the accumulator unchanged.
	assert(lt_crc32(0x12345678u, s, 0) == 0x12345678u);
	return 0;
}
