#include <esp_log.h>

#include "RBCXManager.h"
#include "RBCXUltrasound.h"
#include "rbcx.pb.h"

#define TAG "RBCXUltrasound"

namespace rb {

Ultrasound::Ultrasound()
    : m_index(0)
    , m_lastUs(0)
    , m_lastDistanceMm(0)
    , m_speedOfSound(defaultSpeedOfSound / 100.f)
    , m_measuring(false)
    , m_timeoutTimer(0) {}

Ultrasound::~Ultrasound() {}

void Ultrasound::init(uint8_t index) {
    m_index = index;

    m_timeoutTimer = Timers::get().schedule(0xFFFFFFFF, [this]() -> bool {
        ESP_LOGE(TAG, "Ultrasound %d response timeout!\n", m_index);
        onMeasuringDone(CoprocStat_UltrasoundStat { 0 });
        return true; // return true to prevent cancel, was stopped in onMeasuringDone
    });

    Timers::get().stop(m_timeoutTimer);
}

void Ultrasound::setSpeedOfSound(float speedOfSoundInMetersPerSecond) {
    m_mutex.lock();
    m_speedOfSound = speedOfSoundInMetersPerSecond / 100.f;
    recalculateLastDistanceLocked();
    m_mutex.unlock();
}

void Ultrasound::measureAsync(callback_t callback) {
    std::lock_guard<std::recursive_mutex> ul(m_mutex);

    if (!m_measuring) {
        m_measuring = true;
        Manager::get().sendToCoproc(CoprocReq {
            .which_payload = CoprocReq_ultrasoundReq_tag,
            .payload = {
                .ultrasoundReq = {
                    .utsIndex = m_index,
                    .which_utsCmd = CoprocReq_UltrasoundReq_singlePing_tag,
                },
            },
        });
        Timers::get().reset(m_timeoutTimer, 250);
    }

    if (callback)
        m_callbacks.emplace_back(std::move(callback));
}

uint32_t Ultrasound::measure() {
    if (Timers::get().isOnTimerTask()) {
        ESP_LOGE(TAG,
            "you can't call measure() from the ESP timer task, as it is a "
            "blocking function. Use measureAsync.\n");
        abort();
    }
    std::unique_lock<std::recursive_mutex> ul(m_mutex);
    measureAsync();
    m_measuringDone.wait(ul);
    return m_lastDistanceMm;
}

void Ultrasound::onMeasuringDone(const CoprocStat_UltrasoundStat& result) {
    uint32_t distance;

    {
        std::lock_guard<std::recursive_mutex> ul(m_mutex);

        m_lastUs = result.roundtripMicrosecs;
        recalculateLastDistanceLocked();
        distance = m_lastDistanceMm;
        m_measuring = false;
        Timers::get().stop(m_timeoutTimer);
        m_measuringDone.notify_all();

        for (auto itr = m_callbacks.begin(); itr != m_callbacks.end();) {
            if (!(*itr)(distance)) {
                itr = m_callbacks.erase(itr);
            } else {
                ++itr;
            }
        }

        if (!m_callbacks.empty()) {
            measureAsync();
        }
    }
}

void Ultrasound::recalculateLastDistanceLocked() {
    if (m_lastUs == 0) {
        m_lastDistanceMm = 0;
        return;
    }

    m_lastDistanceMm = uint32_t(m_speedOfSound * (float(m_lastUs) / 10.f)) / 2;
}

};
