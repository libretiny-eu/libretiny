#pragma once

#include <type_traits>

#include "Debug.hpp"
#include "FreeRTOS.h"
#include "queue.h"

template <typename T, UBaseType_t Length> class QueueWrapper {
    static_assert(std::is_trivial<T>::value);

public:
    QueueWrapper()
        : m_handle(nullptr) {}

    ~QueueWrapper() {
        if (m_handle)
            vQueueDelete(m_handle);
    }

    void create() {
        m_handle
            = xQueueCreateStatic(Length, sizeof(T), m_buffer, &m_queueStruct);
    }

    QueueHandle_t handle() const { return m_handle; }

    void reset() { xQueueReset(m_handle); }

    bool push_back(const T& val, TickType_t ticks_to_wait = portMAX_DELAY,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        BaseType_t res;
        if (isInInterrupt()) {
            res = xQueueSendToBackFromISR(
                m_handle, &val, pxHigherPriorityTaskWoken);
        } else {
            res = xQueueSendToBack(m_handle, &val, ticks_to_wait);
        }
        return res == pdPASS;
    }

    bool overwrite(
        const T& val, BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        BaseType_t res;
        if (isInInterrupt()) {
            res = xQueueOverwriteFromISR(
                m_handle, &val, pxHigherPriorityTaskWoken);
        } else {
            res = xQueueOverwrite(m_handle, &val);
        }
        return res == pdPASS;
    }

    bool push_front(const T& val, TickType_t ticks_to_wait = portMAX_DELAY,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        BaseType_t res;
        if (isInInterrupt()) {
            res = xQueueSendToFrontFromISR(
                m_handle, &val, pxHigherPriorityTaskWoken);
        } else {
            res = xQueueSendToFront(m_handle, &val, ticks_to_wait);
        }
        return res == pdPASS;
    }

    bool pop_front(T& result, TickType_t ticks_to_wait = portMAX_DELAY,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr) {
        BaseType_t res;
        if (isInInterrupt()) {
            res = xQueueReceiveFromISR(
                m_handle, &result, pxHigherPriorityTaskWoken);
        } else {
            res = xQueueReceive(m_handle, &result, ticks_to_wait);
        }
        return res == pdTRUE;
    }

private:
    QueueWrapper(const QueueWrapper&) = delete;

    uint8_t m_buffer[sizeof(T) * Length];
    StaticQueue_t m_queueStruct;
    QueueHandle_t m_handle;
};
