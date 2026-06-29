#pragma once

#include <stdint.h>

/// Fast deterministic PRNG - https://en.wikipedia.org/wiki/Xorshift
class XorShift {
    uint32_t m_state;

public:
    XorShift(uint32_t seed = 0xb45d9453)
        : m_state(seed) {}

    uint32_t next() {
        uint32_t x = m_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return m_state = x;
    }
};
