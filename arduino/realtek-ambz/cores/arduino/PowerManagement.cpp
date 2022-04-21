#include "PowerManagement.h"

#ifdef __cplusplus
extern "C" {

#include "freertos_pmu.h"
#include "sys_api.h"

#include "wiring_digital.h"

}
#endif

#include "variant.h"

#if defined(BOARD_RTL8195A)
#define SAVE_LOCK_PIN 18
#elif defined(BOARD_RTL8710)
#define SAVE_LOCK_PIN 7 // PB_1
#else
#define SAVE_LOCK_PIN 18
#endif

bool PowerManagementClass::reservePLL = true;

void PowerManagementClass::setPllReserved(bool reserve) {
    pmu_set_pll_reserved(reserve);
}

void PowerManagementClass::sleep(uint32_t bitflg) {
    if (!safeLock()) {
        pmu_release_wakelock(bitflg);
    }
}

void PowerManagementClass::sleep(void) {
    if (!safeLock()) {
        pmu_release_wakelock(BIT(PMU_OS));
    }
}

void PowerManagementClass::active(uint32_t bitflg) {
    pmu_acquire_wakelock(bitflg);
}

void PowerManagementClass::active(void) {
    pmu_acquire_wakelock(BIT(PMU_OS));
}

void PowerManagementClass::deepsleep(uint32_t duration_ms) {
    if (!safeLock()) {
        deepsleep_ex(DSLEEP_WAKEUP_BY_TIMER, duration_ms);
    }
}

bool PowerManagementClass::safeLock() {
    pinMode(SAVE_LOCK_PIN, INPUT_PULLUP);
    return (digitalRead(SAVE_LOCK_PIN) == 1) ? false : true;
}

void PowerManagementClass::softReset() {
    sys_reset();
}

PowerManagementClass PowerManagement;

