#pragma once
#include <stddef.h>
#include <stdint.h>
// Reflected IEEE CRC-32. Seed with 0xFFFFFFFF, XOR the final result with
// 0xFFFFFFFF. Returns the running (un-finalized) accumulator so callers can
// chain spans. Final CRC = lt_crc32(0xFFFFFFFF, p, n) ^ 0xFFFFFFFF.
uint32_t lt_crc32(uint32_t crc, const void *data, size_t len);
