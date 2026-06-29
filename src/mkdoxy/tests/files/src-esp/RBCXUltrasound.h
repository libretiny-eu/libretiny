#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdint.h>
#include <vector>

#include "RBCXTimers.h"

#include "rbcx.pb.h"

namespace rb {

class Manager;

class Ultrasound {
    friend class Manager;

public:
    //!< Default is speed at 25C, 50%, 101 kPa
    static constexpr float defaultSpeedOfSound = 347.13f;

    typedef std::function<bool(uint32_t)> callback_t;

    uint32_t lastDistanceMm() const { return m_lastDistanceMm; }
    uint32_t lastDurationUs() const { return m_lastUs; }
    bool isLastMeasurementValid() const { return m_lastUs != 0; }

    void setSpeedOfSound(
        float speedOfSoundInMetersPerSecond = defaultSpeedOfSound);
    float getSpeedOfSound() const { return m_speedOfSound; }

    void measureAsync(callback_t callback = nullptr);
    uint32_t measure();

private:
    Ultrasound();
    Ultrasound(const Ultrasound&) = delete;
    ~Ultrasound();

    void init(uint8_t index);

    void recalculateLastDistanceLocked();
    void onMeasuringDone(const CoprocStat_UltrasoundStat& result);

    uint8_t m_index;

    uint32_t m_lastUs;
    uint32_t m_lastDistanceMm;
    float m_speedOfSound;
    bool m_measuring;
    std::vector<callback_t> m_callbacks;
    std::recursive_mutex m_mutex;
    std::condition_variable_any m_measuringDone;
    uint16_t m_timeoutTimer;
};

};
