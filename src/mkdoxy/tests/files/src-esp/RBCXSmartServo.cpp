#include <algorithm>
#include <chrono>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "RBCXManager.h"
#include "RBCXSmartServo.h"

#define TAG "RBCXSmartServoBus"
#define MS_TO_TICKS(ms)                                                        \
    ((portTICK_PERIOD_MS <= ms) ? (ms / portTICK_PERIOD_MS) : 1)

namespace rb {

SmartServoBus::SmartServoBus() {}

void SmartServoBus::install(uint8_t servo_count) {
    if (!m_servos.empty() || servo_count == 0)
        return;

    m_servos.resize(servo_count);

    TaskHandle_t task;
    xTaskCreate(&SmartServoBus::regulatorRoutineTrampoline, "rbservo_reg", 1536,
        this, 2, &task);
    Manager::get().monitorTask(task);

    Angle val;
    for (uint8_t i = 0; i < servo_count; ++i) {
        for (int x = 0; x < 3; ++x) {
            val = pos(i);
            if (!val.isNaN()) {
                break;
            } else {
                ESP_LOGW(
                    TAG, "failed to read servo %d pos, attempt %d", i, x + 1);
            }
        }

        if (val.isNaN()) {
            ESP_LOGE(TAG,
                "failed to read position from servo %d, it will not work!", i);
            continue;
        }

        const uint16_t deg_val = 100 * val.deg();

        m_mutex.lock();
        m_servos[i].current = deg_val;
        m_servos[i].target = deg_val;
        m_mutex.unlock();
    }
}

void SmartServoBus::setId(uint8_t newId, uint8_t destId) {
    // TODO
}

uint8_t SmartServoBus::getId(uint8_t destId) {
    // TODO
    return 0;
}

void SmartServoBus::set(uint8_t id, Angle ang, float speed, float speed_raise) {
    speed = std::max(1.f, std::min(240.f, speed)) / 10.f;
    const uint16_t angle
        = std::max(0.f, std::min(360.f, (float)ang.deg())) * 100;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto& si = m_servos[id];
    if (!si.hasValidCurrent()) {
        const auto cur = pos(id);
        if (cur.isNaN()) {
            ESP_LOGE(TAG, "failed to get servo %d position, can't move it!",
                int(id));
            return;
        }
        const uint16_t deg_val = 100 * cur.deg();
        si.current = deg_val;
        si.target = deg_val;
    }

    if (si.current == angle)
        return;

    if ((si.current > si.target) != (si.current > angle)) {
        si.speed_coef = 0.f;
    }

    si.target = angle;
    si.speed_target = speed;
    si.speed_raise = speed_raise;
}

Angle SmartServoBus::pos(uint8_t id) {
    // TODO
    return Angle::deg(0);
}

Angle SmartServoBus::posOffline(uint8_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& s = m_servos[id];
    if (s.current == 0xFFFF)
        return Angle::nan();
    return Angle::deg(Angle::_T(s.current) / 100.f);
}

void SmartServoBus::limit(uint8_t id, Angle bottom, Angle top) {
    // TODO
}

void SmartServoBus::setAutoStop(uint8_t id, bool enable) {
    m_mutex.lock();
    m_servos[id].auto_stop = enable;
    m_mutex.unlock();
}

void SmartServoBus::regulatorRoutineTrampoline(void* cookie) {
    ((SmartServoBus*)cookie)->regulatorRoutine();
}

void SmartServoBus::regulatorRoutine() {
    const size_t servos_cnt = m_servos.size();

    constexpr uint32_t msPerServo = 30;
    constexpr auto ticksPerServo = MS_TO_TICKS(msPerServo);
    const uint32_t msPerIter = servos_cnt * msPerServo;
    const auto ticksPerIter = MS_TO_TICKS(msPerIter);

    // TODO
    auto queue = xQueueCreate(1, 4 /*sizeof(struct rx_response)*/);
    while (true) {
        const auto tm_iter_start = xTaskGetTickCount();
        for (size_t i = 0; i < servos_cnt; ++i) {
            const auto tm_servo_start = xTaskGetTickCount();
            regulateServo(queue, i, msPerIter);
            const auto diff = xTaskGetTickCount() - tm_servo_start;
            if (diff < ticksPerServo) {
                vTaskDelay(ticksPerServo - diff);
            }
        }

        const auto diff = xTaskGetTickCount() - tm_iter_start;
        if (diff < ticksPerIter) {
            vTaskDelay(ticksPerIter - diff);
        }
    }
}

bool SmartServoBus::regulateServo(
    QueueHandle_t responseQueue, size_t id, uint32_t timeSliceMs) {
    float move_pos_deg;
    auto& s = m_servos[id];

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (s.auto_stop) {
            // TODO
            /*lw::Packet pos_req(id, lw::Command::SERVO_POS_READ);
            send(pos_req, responseQueue, true);
            xQueueReceive(responseQueue, &resp, portMAX_DELAY);
            if (resp.size == 0x08) {
                const float val = (float)((resp.data[6] << 8) | resp.data[5]);
                const int val_int = (val / 1000.f) * 24000.f;
                const int diff = val_int - int(s.current);
                if (abs(diff) > 300) {
                    if (++s.auto_stop_counter > 5) {
                        s.target = val_int + (diff > 0 ? -200 : 200);
                        s.auto_stop_counter = 0;
                    }
                } else if (s.auto_stop_counter != 0) {
                    s.auto_stop_counter = 0;
                }
            }*/
        }

        if (s.current == s.target) {
            return false;
        }

        float speed = s.speed_target;
        if (s.speed_coef < 1.f) {
            s.speed_coef
                = std::min(1.f, s.speed_coef + (s.speed_raise * timeSliceMs));
            speed *= (s.speed_coef * s.speed_coef);
        }

        int32_t dist = abs(int32_t(s.target) - int32_t(s.current));
        dist = std::max(1, std::min(dist, int32_t(speed * timeSliceMs)));
        if (dist > 0) {
            if (s.target < s.current) {
                s.current -= dist;
            } else {
                s.current += dist;
            }
        }

        if (dist <= 0 || s.current == s.target) {
            s.current = s.target;
            s.speed_coef = 0.f;
        }
        move_pos_deg = float(s.current) / 100.f;
    }

    // TODO
    /*const auto pkt = lw::Servo::move(id, Angle::deg(move_pos_deg),
        std::chrono::milliseconds(timeSliceMs - 5));
    send(pkt, responseQueue, false, true);

    if (xQueueReceive(responseQueue, &resp, 500 / portTICK_PERIOD_MS)
        != pdTRUE) {
        ESP_LOGE(TAG, "Response to move packet not received!");
    }*/
    return true;
}

}; // namespace rb
