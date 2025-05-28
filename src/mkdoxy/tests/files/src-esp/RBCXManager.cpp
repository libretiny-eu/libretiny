#include <driver/i2c.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "RBCXBattery.h"
#include "RBCXManager.h"

#include "rbcx.pb.h"

#define TAG "RBCXManager"

#define MOTORS_FAILSAFE_PERIOD_MS 300
#define MAX_COPROC_IDLE_MS 75

namespace rb {

Manager::Manager()
    : m_keepaliveTask(nullptr)
    , m_coprocFwVersion(CoprocStat_VersionStat_init_zero) {}

Manager::~Manager() {}

void Manager::install(
    ManagerInstallFlags flags, BaseType_t managerLoopStackSize) {
    if (m_keepaliveTask != nullptr) {
        ESP_LOGE(TAG,
            "The manager has already been installed, please make sure to "
            "only call install() once!");
        abort();
    }

    for (int i = 0; i < UltrasoundsCount; ++i) {
        m_ultrasounds[i].init(i);
    }

    for (int i = 0; i < StupidServosCount; ++i) {
        m_stupidServos[i].setId(i);
    }

    for (MotorId id = MotorId::M1; id < MotorId::MAX; ++id) {
        m_motors[size_t(id)].setId(id);
    }

    m_motors_last_set = 0;
    if (!(flags & MAN_DISABLE_MOTOR_FAILSAFE)) {
        schedule(MOTORS_FAILSAFE_PERIOD_MS,
            std::bind(&Manager::motorsFailSafe, this));
    }

    const uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, GPIO_NUM_2, GPIO_NUM_0,
        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0));

    m_coprocSemaphore = xSemaphoreCreateBinary();

    // The esp_timer (-> rb::Timers) task runs pinned on core 0, which gets stalled
    // for ~700 ms(?!) when connecting to a WiFi network, so it can't be used
    // for this watchdog.
    xTaskCreate(&Manager::keepaliveRoutine, "rbmanager_keepalive", 1536, this,
        10, &m_keepaliveTask);
    monitorTask(m_keepaliveTask);

    TaskHandle_t task;
    xTaskCreate(&Manager::consumerRoutineTrampoline, "rbmanager_loop",
        managerLoopStackSize, this, 5, &task);
    monitorTask(task);

    sendToCoproc(CoprocReq { .which_payload = CoprocReq_versionReq_tag });
    sendToCoproc(CoprocReq { .which_payload = CoprocReq_getButtons_tag });

    if (xSemaphoreTake(m_coprocSemaphore, pdMS_TO_TICKS(300)) != pdTRUE) {
        ESP_LOGE(TAG,
            "failed to acquire FW version from STM32, message not received in "
            "300ms.\n");
    }

#ifdef RB_DEBUG_MONITOR_TASKS
    schedule(10000, [&]() { return printTasksDebugInfo(); });
#endif
}

/*rb::SmartServoBus& Manager::initSmartServoBus(uint8_t servo_count) {
    m_servos.install(servo_count);
    return m_servos;
}*/

void Manager::consumerRoutineTrampoline(void* cookie) {
    ((Manager*)cookie)->consumerRoutine();
}

void Manager::consumerRoutine() {
    CoprocLinkParser<CoprocStat, &CoprocStat_msg> parser(m_codec);

    while (true) {
        uint8_t byte;
        if (uart_read_bytes(UART_NUM_2, &byte, 1, portMAX_DELAY) != 1) {
            ESP_LOGE(TAG, "Invalid uart read\n");
            continue;
        }

        if (!parser.add(byte))
            continue;

        const auto& msg = parser.lastMessage();
        switch (msg.which_payload) {
        case CoprocStat_buttonsStat_tag:
            m_buttons.setState(msg.payload.buttonsStat);
            break;
        case CoprocStat_ultrasoundStat_tag: {
            const auto& p = msg.payload.ultrasoundStat;
            if (p.utsIndex >= 0 && p.utsIndex < UltrasoundsCount)
                m_ultrasounds[p.utsIndex].onMeasuringDone(p);
            break;
        }
        case CoprocStat_powerAdcStat_tag:
            m_battery.setState(msg.payload.powerAdcStat);
            break;
        case CoprocStat_versionStat_tag:
            m_coprocFwVersion = msg.payload.versionStat;
            xSemaphoreGive(m_coprocSemaphore);
            break;
        case CoprocStat_motorStat_tag: {
            const auto& p = msg.payload.motorStat;
            if (p.motorIndex < (uint32_t)MotorId::MAX) {
                m_motors[p.motorIndex].onMotorStat(p);
            }
            break;
        }

        case CoprocStat_ledsStat_tag:
        case CoprocStat_stupidServoStat_tag:
            // Ignore
            break;
        default:
            printf("Received message of unknown type from stm32: %d\n",
                msg.which_payload);
            break;
        }
    }
}

void Manager::keepaliveRoutine(void* cookie) {
    auto& man = *((Manager*)cookie);

    while (true) {
        if (xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(MAX_COPROC_IDLE_MS))
            == pdFALSE) {
            man.sendToCoproc(CoprocReq {
                .which_payload = CoprocReq_keepalive_tag,
            });
        }
    }
}

void Manager::sendToCoproc(const CoprocReq& msg) {
    m_codecTxMutex.lock();
    const auto len = m_codec.encodeWithHeader(
        &CoprocReq_msg, &msg, m_txBuf, sizeof(m_txBuf));
    if (len > 0) {
        uart_write_bytes(UART_NUM_2, (const char*)m_txBuf, len);
    }
    m_codecTxMutex.unlock();

    xTaskNotify(m_keepaliveTask, 0, eNoAction);
}

void Manager::coprocFwVersionAssert(uint32_t minVersion, const char* name) {
    // Ignore zero version number and pass. Might be the cmd was not received yet,
    // but better than to crash in that case.
    if (m_coprocFwVersion.number == 0)
        return;

    if (minVersion > m_coprocFwVersion.number) {
        printf("\n\nERROR: Please update your STM32 FW, '%s' requires version "
               "0x%06x and you have 0x%06x!\n\n",
            name, minVersion, m_coprocFwVersion.number);
        abort();
    }
}

void Manager::resetMotorsFailSafe() { m_motors_last_set = xTaskGetTickCount(); }

bool Manager::motorsFailSafe() {
    if (m_motors_last_set != 0) {
        const auto now = xTaskGetTickCount();
        if (now - m_motors_last_set
            > pdMS_TO_TICKS(MOTORS_FAILSAFE_PERIOD_MS)) {
            ESP_LOGE(TAG, "Motor failsafe triggered, stopping all motors!");
            for (auto& m : m_motors) {
                m.power(0);
            }
            m_motors_last_set = 0;
        }
    }
    return true;
}

MotorChangeBuilder Manager::setMotors() { return MotorChangeBuilder(); }

void Manager::monitorTask(TaskHandle_t task) {
#ifdef RB_DEBUG_MONITOR_TASKS
    m_tasks_mutex.lock();
    m_tasks.push_back(task);
    m_tasks_mutex.unlock();
#endif
}

#ifdef RB_DEBUG_MONITOR_TASKS
bool Manager::printTasksDebugInfo() {
    std::lock_guard<std::mutex> lock(m_tasks_mutex);

    printf("%16s %5s %5s\n", "Name", "prio", "stack");
    printf("==========================================\n");
    for (auto task : m_tasks) {
        auto stackMark = uxTaskGetStackHighWaterMark(task);
        auto prio = uxTaskPriorityGet(task);
        printf("%16s %5d %5d\n", pcTaskGetTaskName(task), (int)prio,
            (int)stackMark);
    }
    return true;
}
#endif

MotorChangeBuilder::MotorChangeBuilder() {}

MotorChangeBuilder::MotorChangeBuilder(MotorChangeBuilder&& o)
    : m_calls(std::move(o.m_calls)) {}

MotorChangeBuilder::~MotorChangeBuilder() {}

MotorChangeBuilder& MotorChangeBuilder::power(MotorId id, int16_t value) {
    m_calls.emplace_back(
        std::move([=]() { Manager::get().motor(id).power(value); }));
    return *this;
}

MotorChangeBuilder& MotorChangeBuilder::speed(
    MotorId id, int16_t ticksPerSecond) {
    m_calls.emplace_back(
        std::move([=]() { Manager::get().motor(id).speed(ticksPerSecond); }));
    return *this;
}

MotorChangeBuilder& MotorChangeBuilder::pwmMaxPercent(
    MotorId id, int8_t percent) {
    m_calls.emplace_back(
        std::move([=]() { Manager::get().motor(id).pwmMaxPercent(percent); }));
    return *this;
}

MotorChangeBuilder& MotorChangeBuilder::brake(
    MotorId id, uint16_t brakingPower) {
    m_calls.emplace_back(
        std::move([=]() { Manager::get().motor(id).brake(brakingPower); }));
    return *this;
}

MotorChangeBuilder& MotorChangeBuilder::drive(MotorId id,
    int32_t positionRelative, int16_t speedTicksPerSecond,
    Motor::callback_t callback) {

    m_calls.emplace_back(std::move([=]() {
        Manager::get().motor(id).drive(
            positionRelative, speedTicksPerSecond, std::move(callback));
    }));
    return *this;
}

MotorChangeBuilder& MotorChangeBuilder::driveToValue(MotorId id,
    int32_t positionAbsolute, int16_t speedTicksPerSecond,
    Motor::callback_t callback) {
    m_calls.emplace_back(std::move([=]() {
        Manager::get().motor(id).driveToValue(
            positionAbsolute, speedTicksPerSecond, std::move(callback));
    }));
    return *this;
}

void MotorChangeBuilder::set() {
    for (const auto& c : m_calls) {
        c();
    }
    m_calls.clear();
}
};
