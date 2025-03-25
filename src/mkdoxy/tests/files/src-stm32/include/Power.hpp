#pragma once

void powerInit();
void powerPoll();

uint16_t powerBatteryMv();
uint16_t powerBatteryMidMv();
uint16_t powerTemperatureC();
uint16_t powerVrefMv();

static constexpr uint16_t POWER_CALIBRATE_DEFAULT = 0;
static constexpr uint16_t POWER_CALIBRATE_NO_CHANGE = 0xFFFF;
void powerCalibrate(uint16_t vccMv, uint16_t bMidMv, uint16_t vref33Mv,
    uint16_t currentTemperatureC);

void powerShutDown();
