#pragma once

#include "rbcx.pb.h"

namespace rb {

class Manager;

/**
 * \brief Contains the battery state and can control the robot's power.
 */
class Battery {
    friend class Manager;

public:
    static constexpr uint32_t VOLTAGE_MIN = 3300
        * 2; //!< Minimal battery voltage, in mV, at which the robot shuts down
    static constexpr uint32_t VOLTAGE_MAX
        = 4200 * 2; //!< Maximal battery voltage, in mV
    static constexpr uint32_t BATTERY_IN_THRESHOLD = 5000;

    uint32_t pct() const; //!< returns current battery percentage
    //!< returns current battery voltage
    uint32_t voltageMv() const { return m_state.vccMv; }

    //!< returns battery Mid point voltage
    uint32_t bMidVoltageMv() const { return m_state.battMidMv; }

    //!< returns current temperature as measured by the STM32 coprocessor
    int32_t temperatureC() const { return m_state.temperatureC; }

    void shutdown(); //!< shuts the robot's battery power

    //!< Returns true if the Robot's battery power branch is on
    bool isPoweredByBattery() const {
        return voltageMv() > BATTERY_IN_THRESHOLD;
    }

private:
    Battery();
    Battery(const Battery&) = delete;
    ~Battery();

    void setState(const CoprocStat_PowerAdcStat& msg) { m_state = msg; }

    CoprocStat_PowerAdcStat m_state;
};

};
