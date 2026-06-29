#pragma once

#include <chrono>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ratio>

namespace rb {

template <typename T> T clamp(T value, T min, T max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }
    return value;
}

inline void delayMs(int ms) { vTaskDelay(ms / portTICK_PERIOD_MS); }

inline void delay(std::chrono::duration<uint32_t, std::milli> delay) {
    vTaskDelay(delay.count() / portTICK_PERIOD_MS);
}

#define RBCX_ENUM_IMPL_MASK_OPERATORS(T)                                       \
    inline T operator|(T a, T b) {                                             \
        return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));      \
    }                                                                          \
                                                                               \
    inline T operator&(T a, T b) {                                             \
        return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));      \
    }                                                                          \
                                                                               \
    inline T operator^(T a, T b) {                                             \
        return static_cast<T>(static_cast<int>(a) ^ static_cast<int>(b));      \
    }                                                                          \
                                                                               \
    inline T operator|=(T& a, T b) {                                           \
        return static_cast<T>(                                                 \
            reinterpret_cast<int&>(a) |= static_cast<int>(b));                 \
    }                                                                          \
                                                                               \
    inline T operator&=(T& a, T b) {                                           \
        return static_cast<T>(                                                 \
            reinterpret_cast<int&>(a) &= static_cast<int>(b));                 \
    }                                                                          \
                                                                               \
    inline T operator^=(T& a, T b) {                                           \
        return static_cast<T>(                                                 \
            reinterpret_cast<int&>(a) ^= static_cast<int>(b));                 \
    }                                                                          \
                                                                               \
    inline T operator~(T a) { return static_cast<T>(~static_cast<int>(a)); }   \
                                                                               \
    inline T operator<<=(T& a, int b) {                                        \
        return static_cast<T>(                                                 \
            reinterpret_cast<int&>(a) <<= static_cast<int>(b));                \
    }                                                                          \
                                                                               \
    inline T operator>>=(T& a, int b) {                                        \
        return static_cast<T>(                                                 \
            reinterpret_cast<int&>(a) >>= static_cast<int>(b));                \
    }                                                                          \
                                                                               \
    inline T operator<<(T a, int b) {                                          \
        return static_cast<T>(static_cast<int>(a) << static_cast<int>(b));     \
    }                                                                          \
                                                                               \
    inline T operator>>(T a, int b) {                                          \
        return static_cast<T>(static_cast<int>(a) >> static_cast<int>(b));     \
    }                                                                          \
                                                                               \
    T operator+=(T& a, T b) = delete;                                          \
    T operator-=(T& a, T b) = delete;                                          \
    T operator++(T a) = delete;                                                \
    T operator--(T a) = delete;

} // namespace rb
