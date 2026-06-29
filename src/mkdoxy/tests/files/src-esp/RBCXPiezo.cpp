#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>

#include "RBCXManager.h"
#include "RBCXPiezo.h"

#include "rbcx.pb.h"

namespace rb {

Piezo::Piezo()
    : m_on(false) {}

Piezo::~Piezo() {}

void Piezo::setState(bool on) {
    if (m_on == on)
        return;
    m_on = on;

    Manager::get().sendToCoproc(
        CoprocReq { .which_payload = CoprocReq_buzzerReq_tag,
            .payload = { .buzzerReq = {
                             .on = on,
                         } } });
}

};
