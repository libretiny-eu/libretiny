#pragma once

#include <atomic>

namespace rb {

/**
 * \brief Helper class for controlling the piezo.
 */
class Piezo {
    friend class Manager;

public:
    void start() { setState(true); }
    void stop() { setState(false); }

    void setState(bool on);

private:
    Piezo();
    Piezo(const Piezo&) = delete;
    ~Piezo();

    bool m_on;
};
};
