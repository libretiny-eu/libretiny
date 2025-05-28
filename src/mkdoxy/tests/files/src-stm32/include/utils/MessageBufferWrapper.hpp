#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "message_buffer.h"

#include "Debug.hpp"

template <size_t SizeInBytes> class MessageBufferWrapper {
public:
    MessageBufferWrapper()
        : m_handle(nullptr) {}

    ~MessageBufferWrapper() {
        if (m_handle)
            vMessageBufferDelete(m_handle);
    }

    void create() {
        if (m_handle)
            abort();
        m_handle = xMessageBufferCreateStatic(
            SizeInBytes, m_buffer, &m_bufferStruct);
    }

    MessageBufferHandle_t handle() const { return m_handle; }

    bool reset() { return xMessageBufferReset(m_handle) == pdPASS; }

    bool push_back(uint8_t* data, size_t len, TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        size_t res;
        if (isInInterrupt()) {
            res = xMessageBufferSendFromISR(
                m_handle, (void*)data, len, pxHigherPriorityTaskWoken);
        } else {
            res = xMessageBufferSend(m_handle, (void*)data, len, ticks_to_wait);
        }
        return res == len;
    }

    template <typename T>
    bool push_back(const T& val, TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        static_assert(std::is_trivial<T>::value);
        return push_back((uint8_t*)&val, sizeof(T), ticks_to_wait,
            pxHigherPriorityTaskWoken);
    }

    size_t pop_front(uint8_t* dst, size_t maxLen, TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        size_t res;
        if (isInInterrupt()) {
            res = xMessageBufferReceiveFromISR(
                m_handle, (void*)dst, maxLen, pxHigherPriorityTaskWoken);
        } else {
            res = xMessageBufferReceive(
                m_handle, (void*)dst, maxLen, ticks_to_wait);
        }
        return res;
    }

    template <typename T>
    bool pop_front(T& dst, TickType_t ticks_to_wait,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        static_assert(std::is_trivial<T>::value);

        const auto res = pop_front(
            (uint8_t*)dst, sizeof(T), ticks_to_wait, pxHigherPriorityTaskWoken);
        if (res > 0 && res != sizeof(T))
            abort();
        return res != 0;
    }

private:
    uint8_t m_buffer[SizeInBytes];
    StaticMessageBuffer_t m_bufferStruct;
    MessageBufferHandle_t m_handle;
};
