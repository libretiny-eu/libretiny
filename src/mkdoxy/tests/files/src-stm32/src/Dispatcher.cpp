#include "Dispatcher.hpp"
#include "FreeRTOS.h"
#include "utils/Debug.hpp"

#include "Bsp.hpp"
#include "BuzzerController.hpp"
#include "ControlLink.hpp"
#include "Esp32Manager.hpp"
#include "I2cController.hpp"
#include "MotorController.hpp"
#include "Power.hpp"
#include "StupidServoController.hpp"
#include "UltrasoundController.hpp"
#include "queue.h"
#include "rbcx.pb.h"
#include "utils/QueueWrapper.hpp"

static QueueWrapper<CoprocStat, 64> statusQueue;
static QueueWrapper<CoprocReq, 8> requestQueue;

void dispatcherInit() {
    statusQueue.create();
    requestQueue.create();
}

bool dispatcherEnqueueStatus(const CoprocStat& status) {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    auto ok = statusQueue.push_back(status, 0, &pxHigherPriorityTaskWoken);
    if (!ok) {
        DEBUG("Status queue overflow\n");
    }
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    return ok;
}

bool dispatcherEnqueueRequest(const CoprocReq& request) {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    auto ok = requestQueue.push_back(request, 0, &pxHigherPriorityTaskWoken);
    if (!ok) {
        DEBUG("Request queue overflow\n");
    }
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    return ok;
}

static void dispatcherProcessReq(const CoprocReq& request) {
    switch (request.which_payload) {
    case CoprocReq_setLeds_tag: {
        setLeds(request.payload.setLeds.ledsOn);
        const CoprocStat status = {
            .which_payload = CoprocStat_ledsStat_tag,
        };
        controlLinkTx(status);
        break;
    }
    case CoprocReq_getButtons_tag: {
        CoprocStat status = {
            .which_payload = CoprocStat_buttonsStat_tag,
        };
        status.payload.buttonsStat.buttonsPressed
            = CoprocStat_ButtonsEnum(getButtons());
        controlLinkTx(status);
        break;
    }
    case CoprocReq_setStupidServo_tag:
        stupidServoDispatch(request.payload.setStupidServo);
        break;
    case CoprocReq_ultrasoundReq_tag:
        ultrasoundDispatch(request.payload.ultrasoundReq);
        break;
    case CoprocReq_buzzerReq_tag:
        buzzerSetState(request.payload.buzzerReq.on);
        buzzerSetState(request.payload.buzzerReq.on);
        break;
    case CoprocReq_versionReq_tag: {
        CoprocStat status = {
            .which_payload = CoprocStat_versionStat_tag,
        };

        auto& v = status.payload.versionStat;

        static_assert(sizeof(v.revision) == 8);
        static_assert(sizeof(RBCX_VER_REVISION) == 9); // + NULL byte
        memcpy(v.revision, RBCX_VER_REVISION, 8);

        v.dirty = RBCX_VER_DIRTY;
        v.number = RBCX_VER_NUMBER;
        controlLinkTx(status);
        break;
    }
    case CoprocReq_calibratePower_tag: {
        const auto& c = request.payload.calibratePower;
        powerCalibrate(c.vccMv, c.battMidMv, c.vRef33Mv, c.temperatureC);
        break;
    }
    case CoprocReq_shutdownPower_tag:
        powerShutDown();
        break;
    case CoprocReq_motorReq_tag:
        motorDispatch(request.payload.motorReq);
        break;
    case CoprocReq_i2cReq_tag:
        i2cDispatch(request.payload.i2cReq);
        break;
    }
}

void dispatcherPoll() {
    CoprocReq request;
    CoprocStat status;

    if (controlLinkRx(request)) {
        sEsp32Manager.resetWatchdog();
        dispatcherProcessReq(request);
    }

    if (requestQueue.pop_front(request, 0)) {
        dispatcherProcessReq(request);
    }

    if (statusQueue.pop_front(status, 0)) {
        controlLinkTx(status);
    }
}

void dispatcherReset() {
    requestQueue.reset();
    statusQueue.reset();
}
