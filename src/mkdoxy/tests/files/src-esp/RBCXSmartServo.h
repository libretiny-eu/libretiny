#pragma once

#include <FreeRTOS.h>
#include <mutex>
#include <vector>

#include "RBCXAngle.h"
#include "RBCXUtil.h"

namespace rb {

class Manager;
class Encoder;

class SmartServoBus {
    friend class Manager;

public:
    SmartServoBus();
    ~SmartServoBus() {}

    void set(uint8_t id, Angle ang, float speed = 180.f,
        float speed_raise = 0.0015f);
    void limit(uint8_t id, Angle bottom, Angle top);

    Angle pos(uint8_t id);
    Angle posOffline(uint8_t id);

    void setAutoStop(uint8_t id, bool enable = true);

    void setId(uint8_t newId, uint8_t destId = 254);
    uint8_t getId(uint8_t destId = 254);

private:
    SmartServoBus(const SmartServoBus&) = delete;

    void install(uint8_t servo_count);

    static void regulatorRoutineTrampoline(void* cookie);
    void regulatorRoutine();
    bool regulateServo(
        QueueHandle_t responseQueue, size_t id, uint32_t timeSliceMs);

    struct servo_info {
        servo_info() {
            current = 0xFFFF;
            target = 0xFFFF;
            speed_coef = 0.f;
            speed_target = 0.f;
            speed_raise = 0.f;
            auto_stop = false;
            auto_stop_counter = 0;
        }

        bool hasValidCurrent() const { return current != 0xFFFF; }

        float speed_coef;
        float speed_target;
        float speed_raise;
        uint16_t current;
        uint16_t target;
        bool auto_stop;
        uint8_t auto_stop_counter;
    };

    std::vector<servo_info> m_servos;
    std::mutex m_mutex;
};

};
