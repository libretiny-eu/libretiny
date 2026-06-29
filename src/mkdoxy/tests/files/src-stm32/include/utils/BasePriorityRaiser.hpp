#pragma once

#include "FreeRTOS.h"

template <uint32_t TargetIrqPriority> class BasePriorityRaiser {
    static_assert(
        TargetIrqPriority >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
        "The TargetIrqPriority is too high, expected "
        "configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY and lower.");
    static_assert(TargetIrqPriority < configLIBRARY_LOWEST_INTERRUPT_PRIORITY,
        "The TargetIrqPriority is too low, expected "
        "configLIBRARY_LOWEST_INTERRUPT_PRIORITY and higher.");

public:
    BasePriorityRaiser() {}
    ~BasePriorityRaiser() {}

    void lock() {
        vPortSetBASEPRI(TargetIrqPriority << 4);
        __asm volatile("dsb" ::: "memory");
        __asm volatile("isb");
    }

    void unlock() { vPortSetBASEPRI(0); }

private:
    BasePriorityRaiser(const BasePriorityRaiser&) = delete;
};
