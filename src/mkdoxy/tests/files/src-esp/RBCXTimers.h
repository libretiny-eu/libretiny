#pragma once

#include <esp_timer.h>

#include <memory>
#include <mutex>
#include <vector>

namespace rb {

class Manager;
class Timers;

class Timers {
public:
    static constexpr uint16_t INVALID_ID = 0;

    /**
     * \brief  If you don't plan to use FreeRTOS SW timers, call this to free up 2KB of heap
     */
    static void deleteFreeRtOsTimerTask();

    static Timers& get();

    /**
     * \brief Schedule callback to fire after period (in millisecond).
     *
     * Return true from the callback to schedule periodically, false to not (singleshot timer).
     *
     * \param period_ms is period in which will be the schedule callback fired
     * \param callback is a function which will be schedule with the set period.
     * \return timer ID that you can use to cancel the timer.
     */
    uint16_t schedule(uint32_t period_ms, std::function<bool()> callback);

    bool reset(uint16_t id, uint32_t period_ms);
    bool cancel(uint16_t id);
    bool stop(uint16_t id);

    // returns true if the calling code is running in the timer task.
    bool isOnTimerTask() const;

private:
    struct timer_t {
        std::function<bool()> callback;
        esp_timer_handle_t handle;
        uint16_t id;

        void swap(timer_t& o) {
            callback.swap(o.callback);
            std::swap(handle, o.handle);
            std::swap(id, o.id);
        }
    };

    static void timerCallback(void* handleVoid);

    Timers();
    ~Timers();

    void cancelByIdxLocked(size_t idx);
    uint16_t getFreeIdLocked();

    std::vector<timer_t> m_timers;
    std::recursive_mutex m_mutex;
    uint16_t m_id_counter;
};

};
