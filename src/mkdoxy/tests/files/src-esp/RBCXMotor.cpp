#include "RBCXMotor.h"
#include "RBCXManager.h"

#include "rbcx.pb.h"

static constexpr uint32_t minVersionPositional = 0x010100;

namespace rb {

Motor::Motor() {
    m_power = 0;
    m_speed = 0;
    m_position = 0;
    m_mode = MotorMode_POWER;
    m_lastReq = CoprocReq_MotorReq_init_zero;
    m_pwm_max_percent = 100;
}

void Motor::requestInfo(callback_t cb) {
    Manager::get().coprocFwVersionAssert(
        minVersionPositional, "positional motor control (drive)");

    std::lock_guard<std::mutex> l(m_mutex);

    if (m_infoCbs.empty()) {
        Manager::get().sendToCoproc(CoprocReq {
            .which_payload = CoprocReq_motorReq_tag,
            .payload = {
                .motorReq = CoprocReq_MotorReq {
                    .motorIndex = (uint32_t)m_id,
                    .which_motorCmd = CoprocReq_MotorReq_getState_tag,
                },
            },
        });
    }

    if (cb) {
        m_infoCbs.emplace_back(std::move(cb));
    }
}

void Motor::power(int16_t value) {
    sendMotorReq(CoprocReq_MotorReq {
        .motorIndex = (uint32_t)m_id,
        .which_motorCmd = CoprocReq_MotorReq_setPower_tag,
        .motorCmd = {
            .setPower = scale(value),
        },
    });
}

void Motor::speed(int16_t ticksPerSecond) {
    sendMotorReq(CoprocReq_MotorReq {
        .motorIndex = (uint32_t)m_id,
        .which_motorCmd = CoprocReq_MotorReq_setVelocity_tag,
        .motorCmd = {
            .setVelocity = scale(ticksPerSecond),
        },
    });
}

void Motor::brake(uint16_t value) {
    sendMotorReq(CoprocReq_MotorReq {
        .motorIndex = (uint32_t)m_id,
        .which_motorCmd = CoprocReq_MotorReq_setBrake_tag,
        .motorCmd = {
            .setBrake = value,
        },
    });
}

void Motor::pwmMaxPercent(int8_t percent) {
    m_pwm_max_percent = std::min(int8_t(100), std::max(int8_t(0), percent));
}

void Motor::driveToValue(int32_t positionAbsolute, int16_t speedTicksPerSecond,
    callback_t callback) {
    Manager::get().coprocFwVersionAssert(
        minVersionPositional, "positional motor control (drive)");

    sendPositionReq(false,
        CoprocReq_MotorReq_SetPosition {
            .targetPosition = positionAbsolute,
            .runningVelocity = scale(speedTicksPerSecond),
        },
        std::move(callback));
}

void Motor::drive(int32_t positionRelative, int16_t speedTicksPerSecond,
    callback_t callback) {
    Manager::get().coprocFwVersionAssert(
        minVersionPositional, "positional motor control (drive)");
    sendPositionReq(true,
        CoprocReq_MotorReq_SetPosition {
            .targetPosition = positionRelative,
            .runningVelocity = scale(speedTicksPerSecond),
        },
        std::move(callback));
}

void Motor::setConfig(const MotorConfig& conf) {
    Manager::get().coprocFwVersionAssert(
        minVersionPositional, "positional motor control (drive)");

    const CoprocReq coprocReq = {
        .which_payload = CoprocReq_motorReq_tag,
        .payload = {
            .motorReq = CoprocReq_MotorReq {
                .motorIndex = (uint32_t)m_id,
                .which_motorCmd = CoprocReq_MotorReq_setConfig_tag,
                .motorCmd = {
                    .setConfig = conf,
                },
            },
        },
    };

    Manager::get().sendToCoproc(coprocReq);
}

void Motor::setCurrentPosition(int32_t pos) {
    Manager::get().coprocFwVersionAssert(
        minVersionPositional, "positional motor control (drive)");

    std::lock_guard<std::mutex> l(m_mutex);

    m_position = pos;

    const CoprocReq coprocReq = {
        .which_payload = CoprocReq_motorReq_tag,
        .payload = {
            .motorReq = CoprocReq_MotorReq {
                .motorIndex = (uint32_t)m_id,
                .which_motorCmd = CoprocReq_MotorReq_homePosition_tag,
                .motorCmd = {
                    .homePosition = pos,
                },
            },
        },
    };

    Manager::get().sendToCoproc(coprocReq);
}

int16_t Motor::scale(int16_t val) {
    return int32_t(val) * m_pwm_max_percent / 100;
}

void Motor::sendPositionReq(bool additive,
    const CoprocReq_MotorReq_SetPosition& req, callback_t&& callback) {

    {
        std::lock_guard<std::mutex> l(m_mutex);

        Manager::get().resetMotorsFailSafe();
        memset(&m_lastReq, 0, sizeof(CoprocReq_MotorReq));

        m_mode = MotorMode_POSITION;
        m_speed = req.runningVelocity;
        m_positionCb = callback;

        const pb_size_t tag = additive ? CoprocReq_MotorReq_addPosition_tag
                                       : CoprocReq_MotorReq_setPosition_tag;
        const CoprocReq coprocReq = {
            .which_payload = CoprocReq_motorReq_tag,
            .payload = {
                .motorReq = CoprocReq_MotorReq {
                    .motorIndex = (uint32_t)m_id,
                    .which_motorCmd = tag,
                    .motorCmd = {
                        .setPosition = req,
                    },
                },
            },
        };
        Manager::get().sendToCoproc(coprocReq);
    }
}

void Motor::sendMotorReq(const CoprocReq_MotorReq& req) {
    {
        std::lock_guard<std::mutex> l(m_mutex);

        Manager::get().resetMotorsFailSafe();

        if (memcmp(&m_lastReq, &req, sizeof(CoprocReq_MotorReq)) == 0)
            return;
        memcpy(&m_lastReq, &req, sizeof(CoprocReq_MotorReq));

        switch (req.which_motorCmd) {
        case CoprocReq_MotorReq_setPower_tag:
            m_mode = MotorMode_POWER;
            break;
        case CoprocReq_MotorReq_setBrake_tag:
            m_mode = MotorMode_BRAKE;
            break;
        case CoprocReq_MotorReq_setVelocity_tag:
            m_mode = MotorMode_VELOCITY;
            break;
        default:
            abort();
        }

        if (m_mode != MotorMode_POSITION && m_positionCb) {
            m_positionCb = nullptr;
        }

        const CoprocReq coprocReq = {
            .which_payload = CoprocReq_motorReq_tag,
            .payload = {
                .motorReq = req,
            },
        };
        Manager::get().sendToCoproc(coprocReq);
    }
}

void Motor::onMotorStat(const CoprocStat_MotorStat& msg) {
    callback_t positionCb;
    std::vector<callback_t> infoCbs;

    {
        std::lock_guard<std::mutex> l(m_mutex);
        if (m_mode == MotorMode_POSITION && msg.mode == MotorMode_POSITION_IDLE
            && m_positionCb) {
            m_positionCb.swap(positionCb);
        }

        if (!m_infoCbs.empty()) {
            m_infoCbs.swap(infoCbs);
        }

        m_mode = msg.mode;
        m_position = msg.position;
        m_power = msg.power;
        m_speed = msg.velocity;
    }

    if (positionCb) {
        positionCb(*this);
    }

    for (const auto& cb : infoCbs) {
        cb(*this);
    }
}

}; // namespace rb
