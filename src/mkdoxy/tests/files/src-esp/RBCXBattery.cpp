#include <esp_log.h>

#include "RBCXBattery.h"
#include "RBCXManager.h"

#define TAG "RBCXBattery"

namespace rb {

Battery::Battery() {}

Battery::~Battery() {}

void Battery::shutdown() { 
    ESP_LOGW(TAG, "Shutting down.");

    Manager::get().sendToCoproc(CoprocReq {
        .which_payload = CoprocReq_shutdownPower_tag,
    });
 }

uint32_t Battery::pct() const {
    const auto mv = voltageMv();
    if (mv <= VOLTAGE_MIN) {
        return 0;
    } else if (mv >= VOLTAGE_MAX) {
        return 100;
    } else {
        return (float(mv - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN)) * 100.f;
    }
}

};
