#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#include "Bsp.hpp"
#include "utils/Debug.hpp"

#include "Esp32Manager.hpp"

// https://github.com/espressif/esptool/blob/4a1e87d290e9ba2870b4d092baf7b9ae15e4095d/esptool.py#L469

// Bootloader
// [   1441314][src/UsbCdcLink.c:205]: CONTROL_LINE_STATE DTR 0 RTS 1 <-- 0
// [   1441414][src/UsbCdcLink.c:205]: CONTROL_LINE_STATE DTR 1 RTS 1 <-- 1
// [   1441415][src/UsbCdcLink.c:205]: CONTROL_LINE_STATE DTR 1 RTS 0 <-- 2
// [   1441465][src/UsbCdcLink.c:205]: CONTROL_LINE_STATE DTR 0 RTS 0

// Reboot
//                                    (don't care)          DTR 0       <-- 0
// [    444506][src/UsbCdcLink.cpp:205]: CONTROL_LINE_STATE DTR 0 RTS 1 <-- 1
// [    444607][src/UsbCdcLink.cpp:205]: CONTROL_LINE_STATE DTR 0 RTS 0 <-- 2

static constexpr uint32_t Esp32WatchdogTimeoutMs = 150;

Esp32Manager sEsp32Manager;

Esp32Manager::Esp32Manager()
    : m_enPinHolders {}
    , m_queuedReset(RstNormal)
    , m_previousEnEdge(false)
    , m_inBootloader(false)
    , m_watchdogInhibit(false) {}

Esp32Manager::~Esp32Manager() {}

void Esp32Manager::init() {
    pinInit(espEnPin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    m_previousEnEdge = pinRead(espEnPin);
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
        DEBUG("Watchdog inhibited with debugger attached\n");
        m_watchdogInhibit = true;
    }
}

void Esp32Manager::poll() {
    if (m_queuedReset != RstNone) {
        holdReset(EnSwReset);
        releaseReset(EnSwReset, m_queuedReset == RstBootloader);
        m_queuedReset = RstNone;
    }

    if (m_unstrapTimer.poll()) {
        unstrapPins();
    }

    if (m_checkBreakTimer.poll()) {
        if (m_lastRts && !m_lastDtr) {
            holdReset(EnSerialBreaks);
        } else {
            releaseReset(EnSerialBreaks, m_lastDtr && !m_lastRts);
        }
    }

    if (!m_watchdogInhibit && m_watchdogTimer.poll() && !m_inBootloader
        && m_previousEnEdge && m_enPinHolders == 0) {
        DEBUG("Esp32 watchdog timed out, resetting.\n");
        queueReset();
    }
}

void Esp32Manager::holdReset(EnHolderType typ) {
    if (m_enPinHolders == 0) {
        pinInit(espEnPin, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW,
            true);
        pinWrite(espEnPin, 0);
        softReset();
    }
    m_enPinHolders |= (1 << typ);
}

void Esp32Manager::releaseReset(EnHolderType typ, bool strapForBootloader) {
    const uint32_t mask = (1 << typ);
    if (m_enPinHolders == mask) {
        strapPins(strapForBootloader);
        pinInit(espEnPin, GPIO_MODE_IT_RISING_FALLING, GPIO_NOPULL,
            GPIO_SPEED_FREQ_LOW, true);
        m_unstrapTimer.restart(2);
    }
    m_enPinHolders &= ~mask;
}

void Esp32Manager::queueReset(bool bootloader) {
    m_queuedReset = !bootloader ? RstNormal : RstBootloader;
}

void Esp32Manager::strapPins(bool bootloader) {
    //DEBUG("Straping pins, bootloader: %d\n", bootloader);
    pinInit(esp0Pin, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    pinInit(esp12Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
        GPIO_SPEED_FREQ_LOW); // TODO: set pin 12 to OD on RBCX v1.1, v1.0 has HW bug
    pinInit(esp15Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

    pinWrite(esp12Pin, 0); // 3.3v flash
    pinWrite(esp15Pin, 1); // Do not silence bootloader messages

    if (!bootloader) {
        pinWrite(esp0Pin, 1); // normal
    } else {
        pinWrite(esp0Pin, 0); // bootloader

        pinInit(esp2Pin, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
        pinWrite(esp2Pin, 0); // bootloader confirm
    }

    m_inBootloader = bootloader;

    if (bootloader) {
        m_watchdogTimer.stop();
    } else {
        m_watchdogTimer.restart(Esp32WatchdogTimeoutMs * 5);
    }
}

void Esp32Manager::unstrapPins() { reinitEspStrappingPins(); }

void Esp32Manager::onEnRisingInIrq() {
    const bool currentEdge = pinRead(espEnPin);
    if (currentEdge && !m_previousEnEdge && m_enPinHolders == 0) {
        queueReset(m_lastDtr && !m_lastRts);
    } else if (!currentEdge && m_previousEnEdge) {
        softReset();
    }
    m_previousEnEdge = currentEdge;
}

void Esp32Manager::onSerialBreakInIrq(bool dtr, bool rts) {
    m_lastDtr = dtr;
    m_lastRts = rts;
    m_checkBreakTimer.restart(25);
}

void Esp32Manager::resetWatchdog() {
    m_watchdogTimer.restart(Esp32WatchdogTimeoutMs);
}

void Esp32Manager::setWatchdogInhibit(bool inhibit) {
    m_watchdogInhibit = inhibit;
}
