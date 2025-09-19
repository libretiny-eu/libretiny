// I2Cdev library collection - Main I2C device class header file
// Abstracts bit and byte I2C R/W functions into a convenient class
// 6/9/2012 by Jeff Rowberg <jeff@rowberg.net>
// 6/6/2015 by Andrey Voloshin <voloshin@think.in.ua>
//
// Changelog:
//      2015-06-06 - ported to STM32 HAL library from Arduino code

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2013 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/
#pragma once

#include "rbcx.pb.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#include "event_groups.h"

enum I2cEvents : uint32_t {
    I2C_NONE = 0,
    I2C_MPU_TICK = 1,
    I2C_MESSAGE = 2,
};

extern TaskHandle_t i2cTaskHandle;
extern EventGroupHandle_t i2cEventGroup;

void i2cDispatch(const CoprocReq_I2cReq& req);
void i2cReset();

#define I2CDEV_DEFAULT_READ_TIMEOUT 10

uint8_t I2Cdev_init();

uint8_t I2Cdev_Master_Transmit(
    uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_Master_Receive(
    uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_Slave_Transmit(uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_Slave_Receive(uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout);
uint8_t I2Cdev_IsDeviceReady(
    uint16_t DevAddress, uint32_t Trials = 2, uint32_t Timeout = 0);

uint8_t I2Cdev_scan();

uint8_t I2Cdev_readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum,
    uint8_t* data, uint16_t timeout);
uint8_t I2Cdev_readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum,
    uint16_t* data, uint16_t timeout);
uint8_t I2Cdev_readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint8_t* data, uint16_t timeout);
uint8_t I2Cdev_readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint16_t* data, uint16_t timeout);
uint8_t I2Cdev_readByte(
    uint8_t devAddr, uint8_t regAddr, uint8_t* data, uint16_t timeout);
uint8_t I2Cdev_readWord(
    uint8_t devAddr, uint8_t regAddr, uint16_t* data, uint16_t timeout);
uint8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length,
    uint8_t* data, uint16_t timeout);
uint8_t I2Cdev_readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length,
    uint16_t* data, uint16_t timeout);

uint16_t I2Cdev_writeBit(
    uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
uint16_t I2Cdev_writeBitW(
    uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data);
uint16_t I2Cdev_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint8_t data);
uint16_t I2Cdev_writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint16_t data);
uint16_t I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
uint16_t I2Cdev_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data);
uint16_t I2Cdev_writeBytes(
    uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data);
uint16_t I2Cdev_writeWords(
    uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t* data);
