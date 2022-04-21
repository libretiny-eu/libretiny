#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "wdt_api.h"

extern uint32_t ConfigDebugErr;

void wdt_reset() {
    watchdog_refresh();
}

void wdt_enable(uint32_t timeout_ms) {

    uint32_t backup_ConfigDebugErr;

    backup_ConfigDebugErr = ConfigDebugErr;
    ConfigDebugErr = 0x00000000;

    if (timeout_ms > 8000) {
        timeout_ms = 8000;
    }
    watchdog_init(timeout_ms);
    watchdog_start();

    ConfigDebugErr = backup_ConfigDebugErr;
}

void wdt_disable() {
    watchdog_stop();
}

#ifdef __cplusplus
} // extern "C"
#endif
