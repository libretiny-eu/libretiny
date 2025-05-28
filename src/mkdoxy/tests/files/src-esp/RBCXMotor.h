#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "RBCXPinout.h"
#include "RBCXUtil.h"

#include "rbcx.pb.h"

namespace rb {

class Manager;
class MotorChangeBuilder;

class Motor {
    friend class Manager;
    friend class MotorChangeBuilder;

public:
    typedef std::function<void(Motor&)> callback_t;

    MotorId id() const { return m_id; }

    int32_t position() const { return m_position; }

    void requestInfo(callback_t cb);

    /**
     * \brief Set motor power.
     * \param power of the motor <-32768; 32767>
     */
    void power(int16_t value);

    /**
     * \brief Set motor speed
     * \param ticksPerSecond speed of the motor <-32768; 32767>
     */
    void speed(int16_t ticksPerSecond);

    /**
     * \brief Start braking.
     * \param brakingPower braking power in <0, 32767>
     */
    void brake(uint16_t brakingPower);

    /**
     * \brief Limit the maximum PWM value. If you call pwmMaxPercent(70) and then
     * power(100), the motors will spin at 70% of maximum speed. This scales both the power
     * and speed set through Motor.
     * \param pct of the max value <0 - 100>
     */
    void pwmMaxPercent(int8_t percent);

    /**
     * \brief Get current maximum PWM percent value.
     */
    int8_t pwmMaxPercent() const { return m_pwm_max_percent; }

    /**
     * \brief Drive motor to set position (according absolute value).
     */
    void driveToValue(int32_t positionAbsolute, int16_t speedTicksPerSecond,
        callback_t callback = nullptr);

    /**
     * \brief Drive motor to set position (according relative value).
     */
    void drive(int32_t positionRelative, int16_t speedTicksPerSecond,
        callback_t callback = nullptr);

    /**
     * \brief Set the encoder tick counter to some value
     */
    void setCurrentPosition(int32_t pos = 0);

    /**
     * \brief Set configuration, see MotorConfig & STM32 firmware
     */
    void setConfig(const MotorConfig& cfg);

private:
    Motor();
    Motor(const Motor&) = delete;

    void setId(MotorId id) { m_id = id; }

    void sendPositionReq(bool additive,
        const CoprocReq_MotorReq_SetPosition& req, callback_t&& callback);
    void sendMotorReq(const CoprocReq_MotorReq& req);
    void onMotorStat(const CoprocStat_MotorStat& msg);

    int16_t scale(int16_t val);

    callback_t m_positionCb;
    std::vector<callback_t> m_infoCbs;
    CoprocReq_MotorReq m_lastReq;
    std::mutex m_mutex;

    MotorId m_id;
    MotorMode m_mode;
    int32_t m_position;
    int16_t m_power;
    int16_t m_speed;

    int8_t m_pwm_max_percent;
};

} // namespace rb
