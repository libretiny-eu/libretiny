#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_rcc.h"

#include "Bsp.hpp"
#include "BuzzerController.hpp"
#include "Dispatcher.hpp"
#include "Power.hpp"
#include "utils/Debug.hpp"
#include "utils/Flash.hpp"
#include "utils/TaskWrapper.hpp"
#include "utils/TickTimer.hpp"

#include "rbcx.pb.h"

static constexpr uint16_t ThresholdNoBatteryMv = 5000;
static constexpr uint16_t ThresholdPowerOffMv = 6600;

static constexpr uint32_t dispatchStatPeriodMs = 250;
static TickTimer dispatchStatTimer;

static uint16_t rawBattery = 0;
static uint16_t rawMidBattery = 0;
static uint16_t rawInternalRef = 0;
static uint16_t rawTemperature = 0;
static uint16_t vrefMv = 0;

static constexpr int undervoltageShutdownThreshold
    = 5000 / dispatchStatPeriodMs;
static int undervoltageCounter = 0;
static bool undervoltageWarningOn = false;

static constexpr uint16_t tempAvgSlope = 4300;

// Make sure to change the magic when CalibrationData changes.
static constexpr uint32_t calibrationDataMagic = 0x4BCA001;
struct CalibrationData {
    uint32_t magic;
    float batteryCoef;
    float batteryMidCoef;
    uint16_t internalVrefMv;
    uint16_t tempTypicalMv;
    uint16_t tempTypicalAtC;
};

static constexpr CalibrationData calibrationDefaults = {
    .magic = calibrationDataMagic,
    .batteryCoef = 3.4f,
    .batteryMidCoef = 1.56f,
    .internalVrefMv = 1200,
    .tempTypicalMv = 1430,
    .tempTypicalAtC = 25,
};

static CalibrationData calib = calibrationDefaults;

static void adcWait(uint32_t clockCycles) {
    if (clockCycles == 0)
        return;
    uint32_t adcPrescaler = 1;
    const uint32_t APB2prescaler = LL_RCC_GetAPB2Prescaler();
    if (APB2prescaler != 0)
        adcPrescaler
            <<= ((APB2prescaler - RCC_CFGR_PPRE2_DIV2) >> RCC_CFGR_PPRE2_Pos)
            + 1;
    adcPrescaler *= 2
        * ((LL_RCC_GetADCClockSource(RCC_CFGR_ADCPRE) >> RCC_CFGR_ADCPRE_Pos)
            + 1);
    clockCycles *= adcPrescaler;
    while (--clockCycles != 0)
        ;
}

static void loadCalibration() {
    const volatile auto* flashData
        = (volatile CalibrationData*)FLASH_PAGE_POWER_CALIBRATION;

    switch (flashData->magic) {
    case calibrationDataMagic:
        break;
    case 0xFFFFFFFF:
        DEBUG("No calibration data found, using defaults.\n");
        return;
    default:
        DEBUG("Calibration data found, but wrong magic: %08lX, using defaults.",
            flashData->magic);
        return;
    }

    memcpy(&calib, (void*)flashData, sizeof(CalibrationData));

    DEBUG(
        "Loaded calibration data - VCC:%f BMID:%f VREFINT:%dmV temp:%dmV@%dC\n",
        calib.batteryCoef, calib.batteryMidCoef, calib.internalVrefMv,
        calib.tempTypicalMv, calib.tempTypicalAtC);
}

void powerInit() {
    pinInit(
        batteryVoltagePin, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);
    pinInit(batteryMiddleVoltagePin, GPIO_MODE_ANALOG, GPIO_NOPULL,
        GPIO_SPEED_FREQ_LOW);

    loadCalibration();

    LL_ADC_InitTypeDef cfg;
    LL_ADC_StructInit(&cfg);
    cfg.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    cfg.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
    if (LL_ADC_Init(auxiliaryAdc, &cfg) != SUCCESS)
        abort();

    LL_ADC_REG_InitTypeDef cfgReg;
    LL_ADC_REG_StructInit(&cfgReg);
    cfgReg.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
    cfgReg.SequencerLength = LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS;
    cfgReg.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    if (LL_ADC_REG_Init(auxiliaryAdc, &cfgReg) != SUCCESS)
        abort();

    LL_ADC_INJ_InitTypeDef cfgInj;
    LL_ADC_INJ_StructInit(&cfgInj);
    cfgInj.TriggerSource = LL_ADC_INJ_TRIG_SOFTWARE;
    cfgInj.TrigAuto = LL_ADC_INJ_TRIG_FROM_GRP_REGULAR;
    if (LL_ADC_INJ_Init(auxiliaryAdc, &cfgInj) != SUCCESS)
        abort();

    LL_ADC_SetCommonPathInternalCh(ADC12_COMMON, LL_ADC_PATH_INTERNAL_VREFINT);

    LL_ADC_Enable(auxiliaryAdc);
    adcWait(LL_ADC_DELAY_ENABLE_CALIB_ADC_CYCLES);
    LL_ADC_StartCalibration(auxiliaryAdc);
    while (LL_ADC_IsCalibrationOnGoing(auxiliaryAdc))
        ;

    LL_ADC_SetChannelSamplingTimeFix(
        auxiliaryAdc, batteryVoltageAdcChannel, LL_ADC_SAMPLINGTIME_1CYCLE_5);
    LL_ADC_SetChannelSamplingTimeFix(auxiliaryAdc,
        batteryMiddleVoltageAdcChannel, LL_ADC_SAMPLINGTIME_1CYCLE_5);
    LL_ADC_SetChannelSamplingTimeFix(
        auxiliaryAdc, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_239CYCLES_5);
    LL_ADC_SetChannelSamplingTimeFix(auxiliaryAdc, LL_ADC_CHANNEL_TEMPSENSOR,
        LL_ADC_SAMPLINGTIME_239CYCLES_5);

    auxiliaryAdc->JSQR = ADC_JSQR_JL
        | (batteryVoltageAdcChannel & ADC_JSQR_JSQ1)
            << (ADC_JSQR_JSQ2_Pos * ((batteryVoltageAdcRank - 1) & 3))
        | (LL_ADC_CHANNEL_VREFINT & ADC_JSQR_JSQ1)
            << (ADC_JSQR_JSQ2_Pos * ((internalReferenceVoltageAdcRank - 1) & 3))
        | (batteryMiddleVoltageAdcChannel & ADC_JSQR_JSQ1)
            << (ADC_JSQR_JSQ2_Pos * ((batteryMiddleVoltageAdcRank - 1) & 3))
        | (LL_ADC_CHANNEL_TEMPSENSOR & ADC_JSQR_JSQ1)
            << (ADC_JSQR_JSQ2_Pos * ((temperatureAdcRank - 1) & 3));

    LL_ADC_EnableIT_JEOS(auxiliaryAdc);
    HAL_NVIC_SetPriority(
        auxiliaryAndMotorAdcIRQn, auxiliaryAndMotorAdcIrqPrio, 0);
    HAL_NVIC_EnableIRQ(auxiliaryAndMotorAdcIRQn);

    LL_ADC_REG_StartConversionSWStart(auxiliaryAdc);
    LL_ADC_INJ_StartConversionSWStart(auxiliaryAdc);

    dispatchStatTimer.restart(dispatchStatPeriodMs);
}

extern "C" void AUXILIARY_AND_MOTOR_ADC_IRQ_HANDLER() {
    if (LL_ADC_IsEnabledIT_JEOS(auxiliaryAdc)
        && LL_ADC_IsActiveFlag_JEOS(auxiliaryAdc)) {

        rawBattery = LL_ADC_INJ_ReadConversionData12(
            auxiliaryAdc, batteryVoltageAdcRank);
        rawMidBattery = LL_ADC_INJ_ReadConversionData12(
            auxiliaryAdc, batteryMiddleVoltageAdcRank);
        rawTemperature
            = LL_ADC_INJ_ReadConversionData12(auxiliaryAdc, temperatureAdcRank);
        rawInternalRef = LL_ADC_INJ_ReadConversionData12(
            auxiliaryAdc, internalReferenceVoltageAdcRank);

        vrefMv = calib.internalVrefMv
            * __LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B) / rawInternalRef;

        LL_ADC_ClearFlag_JEOS(auxiliaryAdc);
    }
    HAL_NVIC_ClearPendingIRQ(auxiliaryAndMotorAdcIRQn);
}

uint16_t powerBatteryMv() {
    return __LL_ADC_CALC_DATA_TO_VOLTAGE(
               vrefMv, rawBattery, LL_ADC_RESOLUTION_12B)
        * calib.batteryCoef;
}

uint16_t powerBatteryMidMv() {
    return __LL_ADC_CALC_DATA_TO_VOLTAGE(
               vrefMv, rawMidBattery, LL_ADC_RESOLUTION_12B)
        * calib.batteryMidCoef;
}

uint16_t powerTemperatureC() {
    return __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(tempAvgSlope,
        calib.tempTypicalMv, calib.tempTypicalAtC, vrefMv, rawTemperature,
        LL_ADC_RESOLUTION_12B);
}

uint16_t powerVrefMv() { return vrefMv; }

void powerCalibrate(uint16_t vccMv, uint16_t bMidMv, uint16_t vref33Mv,
    uint16_t currentTemperatureC) {

    while (vrefMv == 0)
        ;

    portTICK_TYPE_ENTER_CRITICAL();

    if (vref33Mv == POWER_CALIBRATE_DEFAULT) {
        calib.internalVrefMv = calibrationDefaults.internalVrefMv;
    } else if (vref33Mv != POWER_CALIBRATE_NO_CHANGE) {
        vrefMv = vref33Mv;
        calib.internalVrefMv = vrefMv * rawInternalRef
            / __LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B);
    }

    if (vccMv == POWER_CALIBRATE_DEFAULT) {
        calib.batteryCoef = calibrationDefaults.batteryCoef;
    } else if (vccMv != POWER_CALIBRATE_NO_CHANGE) {
        calib.batteryCoef = float(vccMv)
            / float(__LL_ADC_CALC_DATA_TO_VOLTAGE(
                vrefMv, rawBattery, LL_ADC_RESOLUTION_12B));
    }

    if (bMidMv == POWER_CALIBRATE_DEFAULT) {
        calib.batteryMidCoef = calibrationDefaults.batteryMidCoef;
    } else if (bMidMv != POWER_CALIBRATE_NO_CHANGE) {
        calib.batteryMidCoef = float(bMidMv)
            / float(__LL_ADC_CALC_DATA_TO_VOLTAGE(
                vrefMv, rawMidBattery, LL_ADC_RESOLUTION_12B));
    }

    if (currentTemperatureC == POWER_CALIBRATE_DEFAULT) {
        calib.tempTypicalMv = calibrationDefaults.tempTypicalMv;
        calib.tempTypicalAtC = calibrationDefaults.tempTypicalAtC;
    } else if (currentTemperatureC != POWER_CALIBRATE_NO_CHANGE) {
        calib.tempTypicalMv = __LL_ADC_CALC_DATA_TO_VOLTAGE(
            vrefMv, rawTemperature, LL_ADC_RESOLUTION_12B);
        calib.tempTypicalAtC = currentTemperatureC;
    }

    portTICK_TYPE_EXIT_CRITICAL();

    flashWritePage(FLASH_PAGE_POWER_CALIBRATION, (uint8_t*)&calib,
        sizeof(CalibrationData));

    DEBUG("New coefs set, internalRef: %dmv bat: %f mid %f temp %d@%d\n",
        calib.internalVrefMv, calib.batteryCoef, calib.batteryMidCoef,
        calib.tempTypicalMv, calib.tempTypicalAtC);
}

void powerShutDown() {
    DEBUG("Shutting down battery power...\n");

    if (pinRead(usbBusDetectionPin)) {
        pinWrite(powerPin, 0);
        return;
    }

    vTaskDelay(1);

    __disable_irq();

    pinWrite(powerPin, 0);

    // Blink red LED "pretty fast"
    uint32_t leds = 0;
    while (true) {
        leds ^= CoprocReq_LedsEnum_L3;
        setLeds(leds);

        for (volatile int i = 0; i < 200000; i++)
            ;
    }
}

static void checkBatteryVoltage(uint16_t vccMv) {
    if (vccMv < ThresholdNoBatteryMv) {
        if (undervoltageWarningOn) {
            undervoltageWarningOn = false;
            pinWrite(led3Pin, false);
            buzzerSetState(false);
        }
        return;
    }

    if (vccMv < ThresholdPowerOffMv && pinRead(powerPin)) {
        ++undervoltageCounter;
        undervoltageWarningOn = !undervoltageWarningOn;
        pinWrite(led3Pin, undervoltageWarningOn);
        buzzerSetState(undervoltageWarningOn);
        DEBUG("Battery voltage is low, %dmv.\n", vccMv);

        if (undervoltageCounter > undervoltageShutdownThreshold) {
            undervoltageCounter = 0;
            DEBUG("Battery voltage low, %dmv. Shutting down.\n", vccMv);
            powerShutDown();
        }
    } else if (undervoltageCounter > 0) {
        --undervoltageCounter;
        if (undervoltageWarningOn) {
            undervoltageWarningOn = false;
            pinWrite(led3Pin, false);
            buzzerSetState(false);
        }
    }
}

void powerPoll() {
    if (dispatchStatTimer.poll()) {
        const auto vccMv = powerBatteryMv();
        checkBatteryVoltage(vccMv);

        CoprocStat status = {
            .which_payload = CoprocStat_powerAdcStat_tag,
        };
        status.payload.powerAdcStat.vccMv = vccMv;
        status.payload.powerAdcStat.battMidMv = powerBatteryMidMv();
        status.payload.powerAdcStat.temperatureC = powerTemperatureC();
        dispatcherEnqueueStatus(status);

        dispatchStatTimer.restart(dispatchStatPeriodMs);
    }
}
