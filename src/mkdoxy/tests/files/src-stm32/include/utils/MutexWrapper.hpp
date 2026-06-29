#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

class MutexWrapper {
public:
    MutexWrapper()
        : m_handle(nullptr) {}

    ~MutexWrapper() {
        if (m_handle)
            vSemaphoreDelete(m_handle);
    }

    void create() {
        if (m_handle)
            abort();

        m_handle = xSemaphoreCreateMutexStatic(&m_buffer);
    }

    SemaphoreHandle_t native_handle() const { return m_handle; }

    void lock() { xSemaphoreTake(m_handle, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(m_handle); }

private:
    SemaphoreHandle_t m_handle;
    StaticSemaphore_t m_buffer;
};
