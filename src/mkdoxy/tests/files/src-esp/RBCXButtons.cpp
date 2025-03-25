#include "RBCXButtons.h"

namespace rb {

Buttons::Buttons()
    : m_buttonsSet {} {}

Buttons::~Buttons() {}

void Buttons::setState(const CoprocStat_ButtonsStat& msg) {
    const uint32_t diff = m_buttonsSet ^ (ButtonId)msg.buttonsPressed;
    if (diff == 0)
        return;

    m_buttonsSet = (ButtonId)msg.buttonsPressed;

    std::lock_guard<std::recursive_mutex> l(m_mutex);
    for (uint32_t i = 0; i < Count; ++i) {
        if ((diff & (1 << i)) == 0)
            continue;

        for (size_t x = 0; x < m_callbacks.size();) {
            const auto id = ButtonId(1 << i);
            if (!m_callbacks[x](id, byId(id))) {
                m_callbacks.erase(m_callbacks.begin() + x);
            } else {
                ++x;
            }
        }
    }
}

void Buttons::onChange(callback_t callback) {
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    m_callbacks.emplace_back(std::move(callback));
}
};
