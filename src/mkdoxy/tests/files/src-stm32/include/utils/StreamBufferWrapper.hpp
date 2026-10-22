#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include "Debug.hpp"

template <size_t SizeInBytes> class StreamBufferWrapper {
public:
    StreamBufferWrapper()
        : m_handle(nullptr) {}

    ~StreamBufferWrapper() {
        if (m_handle)
            vStreamBufferDelete(m_handle);
    }

    void create(size_t xTriggerLevelBytes = 1) {
        if (m_handle)
            abort();
        if (xTriggerLevelBytes > SizeInBytes)
            abort();

        m_handle = xStreamBufferCreateStatic(
            SizeInBytes, xTriggerLevelBytes, m_buffer, &m_bufferStruct);
    }

    StreamBufferHandle_t handle() const { return m_handle; }

    bool reset() { return xStreamBufferReset(m_handle) == pdPASS; }

    size_t freeSpace() const { return xStreamBufferSpacesAvailable(m_handle); }
    size_t available() const { return xStreamBufferBytesAvailable(m_handle); }

    size_t write(const uint8_t* const data, size_t len,
        TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        size_t res;
        if (isInInterrupt()) {
            res = xStreamBufferSendFromISR(
                m_handle, (void*)data, len, pxHigherPriorityTaskWoken);
        } else {
            res = xStreamBufferSend(m_handle, (void*)data, len, ticks_to_wait);
        }
        return res;
    }

    size_t read(uint8_t* dst, size_t maxLen, TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        size_t res;
        if (isInInterrupt()) {
            res = xStreamBufferReceiveFromISR(
                m_handle, (void*)dst, maxLen, pxHigherPriorityTaskWoken);
        } else {
            res = xStreamBufferReceive(
                m_handle, (void*)dst, maxLen, ticks_to_wait);
        }
        return res;
    }

private:
    uint8_t m_buffer[SizeInBytes];
    StaticStreamBuffer_t m_bufferStruct;
    StreamBufferHandle_t m_handle;
};
