#pragma once

#include <mutex>

#include "RBCXUtil.h"

#include "rbcx.pb.h"

namespace rb {

/**
 * \brief Helper class for controlling the LEDs connected to the expander.
 */

enum LedId {
    L1 = CoprocReq_LedsEnum_L1,
    L2 = CoprocReq_LedsEnum_L2,
    L3 = CoprocReq_LedsEnum_L3,
    L4 = CoprocReq_LedsEnum_L4,

    Green = L1,
    Yellow = L2,
    Red = L3,
    Blue = L4,

    All = L1 | L2 | L3 | L4,
};

RBCX_ENUM_IMPL_MASK_OPERATORS(LedId)

class Leds {
    friend class Manager;

public:
    void red(bool on = true); //!< Set the red LED state
    void yellow(bool on = true); //!< Set the yellow LED state
    void green(bool on = true); //!< Set the green LED state
    void blue(bool on = true); //!< Set the blue LED state
    void byId(LedId id, bool on = true); //!< Set led styte by LedId

private:
    Leds();
    Leds(const Leds&) = delete;
    ~Leds();

    LedId m_ledsOn;
    std::mutex m_mutex;
};

};
