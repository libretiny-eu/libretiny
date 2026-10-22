#pragma once

#include "utils/TickTimer.hpp"

class Esp32Manager {
    Esp32Manager(const Esp32Manager&) = delete;

public:
    Esp32Manager();
    ~Esp32Manager();

    void init();
    void poll();

    void queueReset(bool bootloader = false);

    void onEnRisingInIrq();
    void onSerialBreakInIrq(bool dtr, bool rst);

    bool isInBootloader() const { return m_inBootloader; }
    void resetWatchdog();
    void setWatchdogInhibit(bool inhibit);

private:
    enum EnHolderType {
        EnSerialBreaks = 0,
        EnSwReset = 1,
    };

    enum QueuedReset {
        RstNone = 0,
        RstNormal = 1,
        RstBootloader = 2,
    };

    void holdReset(EnHolderType typ);
    void releaseReset(EnHolderType typ, bool strapForBootloader = false);

    void strapPins(bool bootloader);
    void unstrapPins();

    TickTimer m_unstrapTimer;
    TickTimer m_checkBreakTimer;
    TickTimer m_watchdogTimer;

    uint32_t m_enPinHolders;

    QueuedReset m_queuedReset;
    bool m_previousEnEdge;
    bool m_lastRts;
    bool m_lastDtr;
    bool m_inBootloader;
    bool m_watchdogInhibit;
};

extern Esp32Manager sEsp32Manager;
