#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

class TickTimer {
    static constexpr uint32_t MaxDelay = UINT32_MAX / 2;

public:
    TickTimer()
        : m_timeoutAt(0) {}
    ~TickTimer() {}

    bool isActive() const { return m_timeoutAt != 0; }

    void stop() { m_timeoutAt = 0; }

    void restart(uint32_t delayMs) {
        if (pdMS_TO_TICKS(delayMs) > MaxDelay) {
            abort();
        }

        const auto now = xTaskGetTickCount();
        m_timeoutAt = now + pdMS_TO_TICKS(delayMs);
        if (m_timeoutAt == 0)
            ++m_timeoutAt;
    }

    bool poll() {
        if (m_timeoutAt == 0)
            return false;

        const uint32_t now = xTaskGetTickCount();
        if (now < m_timeoutAt)
            return false;

        // m_timeoutAt wrapped around, but xTaskGetTickCount didn't yet
        if (now - m_timeoutAt > MaxDelay)
            return false;
        m_timeoutAt = 0;
        return true;
    }

private:
    uint32_t m_timeoutAt;
};
