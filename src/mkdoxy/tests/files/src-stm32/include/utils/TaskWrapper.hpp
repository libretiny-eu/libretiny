#pragma once

#include <functional>

#include "FreeRTOS.h"
#include "task.h"

template <int StackSizeBytes> class TaskWrapper {
public:
    TaskWrapper()
        : m_handle(nullptr) {}

    ~TaskWrapper() { abort(); }

    bool start(
        const char* name, UBaseType_t priority, std::function<void()>&& func) {
        if (m_handle)
            return false;
        m_func = std::move(func);
        m_handle = xTaskCreateStatic(&trampoline, name,
            StackSizeBytes / sizeof(StackType_t), &m_func, priority, m_stack,
            &m_taskStruct);
        return m_handle != nullptr;
    }

    TaskHandle_t handle() const { return m_handle; }

private:
    TaskWrapper(const TaskWrapper&) = delete;

    static void trampoline(void* arg) {
        const auto* func = (std::function<void()>*)arg;
        (*func)();
    }

    StackType_t m_stack[StackSizeBytes / sizeof(StackType_t)];
    StaticTask_t m_taskStruct;
    std::function<void()> m_func;
    TaskHandle_t m_handle;
};
