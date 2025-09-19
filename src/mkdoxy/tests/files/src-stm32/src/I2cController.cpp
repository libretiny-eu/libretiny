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
#include "I2cController.hpp"
#include "Bsp.hpp"
#include "MpuController.hpp"
#include "OledController.hpp"
#include "utils/Debug.hpp"

#include "utils/MutexWrapper.hpp"
#include "utils/QueueWrapper.hpp"
#include "utils/TaskWrapper.hpp"
#include "timers.h"

// #include "event_groups.h"
#include <mutex>

static TaskWrapper<1024> i2cTask;
TaskHandle_t i2cTaskHandle;

static QueueWrapper<CoprocReq_I2cReq, 16> i2cQueue;
static xTaskHandle i2cCallingTask;
static MutexWrapper i2cMutex;

EventGroupHandle_t i2cEventGroup;
static StaticEventGroup_t i2cEventBuffer;

// Hold pointer to inited HAL I2C device
I2C_HandleTypeDef I2Cdev_hi2c;

void i2cDispatch(const CoprocReq_I2cReq& req) {
    if (i2cQueue.push_back(req, 0)) {
        xEventGroupSetBits(i2cEventGroup, I2C_MESSAGE);
    } else {
        DEBUG("I2c queue overflow\n");
    }
}

/** Default timeout value for read operations.
 * Set this to 0 to disable timeout detection.
 */
uint16_t I2Cdev_readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;

uint8_t I2Cdev_init() {
    I2Cdev_hi2c.Instance = I2C1;
    I2Cdev_hi2c.Init.ClockSpeed = 400000;
    I2Cdev_hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    I2Cdev_hi2c.Init.OwnAddress1 = 0;
    I2Cdev_hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2Cdev_hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2Cdev_hi2c.Init.OwnAddress2 = 0;
    I2Cdev_hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2Cdev_hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    auto init = HAL_I2C_Init(&I2Cdev_hi2c);

    if (init == HAL_OK) {
        i2cQueue.create();
        i2cMutex.create();
        i2cEventGroup = xEventGroupCreateStatic(&i2cEventBuffer);

        i2cTask.start("i2c", i2cPrio, []() {
            while (true) {
                CoprocReq_I2cReq req;

                EventBits_t eventsBit = I2C_NONE;
                eventsBit = xEventGroupWaitBits(
                    i2cEventGroup, 0xFF, pdTRUE, 0, portMAX_DELAY);

                if (eventsBit & I2C_MPU_TICK) {
                    mpuTick();
                    // DEBUG("SEND MPU\n");
                }

                while (i2cQueue.pop_front(req, 0)) {
                    switch (req.which_payload) {
                    case CoprocReq_I2cReq_oledReq_tag:
                        oledDispatch(req.payload.oledReq);
                        break;
                    case CoprocReq_I2cReq_mpuReq_tag:
                        mpuDispatch(req.payload.mpuReq);
                        // DEBUGLN("MPU Req %d", req.payload.mpuReq.which_mpuCmd);
                        break;
                    }
                }
                // vTaskDelay(pdMS_TO_TICKS(10));
            }
        });
        i2cTaskHandle = i2cTask.handle();
    } else {
        DEBUG("Error I2c Init\n");
    }
}

void i2cNotify() {
    BaseType_t woken = 0;
    xTaskNotifyFromISR(i2cCallingTask, 0, eNoAction, &woken);
    portYIELD_FROM_ISR(woken);
}

extern "C" void I2C1_EV_IRQHandler() { HAL_I2C_EV_IRQHandler(&I2Cdev_hi2c); }
extern "C" void I2C1_ER_IRQHandler() { HAL_I2C_ER_IRQHandler(&I2Cdev_hi2c); }

extern "C" __weak void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    i2cNotify();
}

extern "C" __weak void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    i2cNotify();
}

extern "C" __weak void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    i2cNotify();
}

extern "C" __weak void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    i2cNotify();
}

extern "C" __weak void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c) {
    i2cNotify();
}

uint8_t i2cWait(HAL_StatusTypeDef beginStatus, uint32_t tout) {}

template <typename F> uint8_t i2cWrap(F fun, uint32_t Timeout) {
    uint16_t tout = Timeout > 0 ? Timeout : I2CDEV_DEFAULT_READ_TIMEOUT;
    std::scoped_lock lock(i2cMutex);
    i2cCallingTask = xTaskGetCurrentTaskHandle();
    auto beginStatus = fun();

    if (beginStatus == HAL_OK) {
        auto ok = xTaskNotifyWait(0, ~0, nullptr, pdMS_TO_TICKS(tout));

        if (!ok) {
            DEBUG("I2C timeout\n");
            HAL_I2C_Master_Abort_IT(&I2Cdev_hi2c, 0);
        }
        return ok;
    } else {
        HAL_I2C_Master_Abort_IT(&I2Cdev_hi2c, 0);
        return 0;
    }
}

void i2cReset() { 
    i2cQueue.reset();        
}

/* Rewrited original functions */
uint8_t I2Cdev_Master_Transmit(
    uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    return i2cWrap(
        [=]() {
            auto beginStatus = HAL_I2C_Master_Transmit_IT(
                &I2Cdev_hi2c, DevAddress << 1, pData, Size);

            if (beginStatus != HAL_OK) {
                DEBUG("I2C ERR - Tx %d bytes, ret: %d\n", Size, beginStatus);
            }
            return beginStatus;
        },
        Timeout);
}

uint8_t I2Cdev_Master_Receive(
    uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    return i2cWrap(
        [=]() {
            auto beginStatus = HAL_I2C_Master_Receive_IT(
                &I2Cdev_hi2c, DevAddress << 1, pData, Size);

            if (beginStatus != HAL_OK) {
                DEBUG("I2C ERR - Rx %d bytes, ret: %d\n", Size, beginStatus);
            }
            return beginStatus;
        },
        Timeout);
}

uint8_t I2Cdev_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    return i2cWrap(
        [=]() {
            auto beginStatus = HAL_I2C_Mem_Write_IT(&I2Cdev_hi2c,
                DevAddress << 1, MemAddress, MemAddSize, pData, Size);

            if (beginStatus != HAL_OK) {
                DEBUG("I2C ERR - Mem write %d bytes, ret: %d\n", Size, beginStatus);
            }
            return beginStatus;
        },
        Timeout);
}

uint8_t I2Cdev_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    return i2cWrap(
        [=]() {
            auto beginStatus = HAL_I2C_Mem_Read_IT(&I2Cdev_hi2c,
                DevAddress << 1, MemAddress, MemAddSize, pData, Size);

            if (beginStatus != HAL_OK) {
                DEBUG("I2C ERR - Mem read %d bytes, ret: %d\n", Size, beginStatus);
            }
            return beginStatus;
        },
        Timeout);
}

uint8_t I2Cdev_IsDeviceReady(
    uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) {
    uint16_t tout = Timeout > 0 ? Timeout : I2CDEV_DEFAULT_READ_TIMEOUT;
    return HAL_I2C_IsDeviceReady(&I2Cdev_hi2c, DevAddress << 1, Trials, tout)
        == HAL_OK;
}
/* Rewrited original functions */

uint8_t I2Cdev_scan() {
    uint8_t counter = 0;
    for (int range = 1; range <= 254; range++) {
        if (I2Cdev_IsDeviceReady(range) != 0) {
            DEBUG("I2Cdev_scan[%d] ready: %#04x (%d)\n", counter, range, range);
            counter++;
        }
    }
    return counter;
}

/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum,
    uint8_t* data, uint16_t timeout) {
    uint8_t b;
    uint8_t count = I2Cdev_readByte(devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read a single bit from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-15)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum,
    uint16_t* data, uint16_t timeout) {
    uint16_t b;
    uint8_t count = I2Cdev_readWord(devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint8_t* data, uint16_t timeout) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = I2Cdev_readByte(devAddr, regAddr, &b, timeout)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Read multiple bits from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-15)
 * @param length Number of bits to read (not more than 16)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (1 = success, 0 = failure, -1 = timeout)
 */
uint8_t I2Cdev_readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint16_t* data, uint16_t timeout) {
    // 1101011001101001 read byte
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    //    010           masked
    //           -> 010 shifted
    uint8_t count;
    uint16_t w;
    if ((count = I2Cdev_readWord(devAddr, regAddr, &w, timeout)) != 0) {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        w &= mask;
        w >>= (bitStart - length + 1);
        *data = w;
    }
    return count;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readByte(
    uint8_t devAddr, uint8_t regAddr, uint8_t* data, uint16_t timeout) {
    return I2Cdev_readBytes(devAddr, regAddr, 1, data, timeout);
}

/** Read single word from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for word value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readWord(
    uint8_t devAddr, uint8_t regAddr, uint16_t* data, uint16_t timeout) {
    return I2Cdev_readWords(devAddr, regAddr, 1, data, timeout);
}

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Number of bytes read (-1 indicates failure)
 */
uint8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length,
    uint8_t* data, uint16_t timeout) {
    uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;
    I2Cdev_Master_Transmit(devAddr, &regAddr, 1, tout);
    uint8_t ret = I2Cdev_Master_Receive(devAddr, data, length, tout);
    if (ret != 0) {
        return length;
    }
    return -1;
}

// uint8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout)
// {
//     uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;

//     HAL_I2C_Master_Transmit(&I2Cdev_hi2c, devAddr << 1, &regAddr, 1, tout);
//     if (HAL_I2C_Master_Receive(&I2Cdev_hi2c, devAddr << 1, data, length, tout) == HAL_OK) {
//         DEBUG("I2Cdev_readBytes %d\n", length);
//         return length;
//     }
//     return -1;
// }

/** Read multiple words from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of words to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Number of words read (-1 indicates failure)
 */
uint8_t I2Cdev_readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length,
    uint16_t* data, uint16_t timeout) {
    uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;

    I2Cdev_Master_Transmit(devAddr, &regAddr, 1, tout);
    if (I2Cdev_Master_Receive(devAddr, (uint8_t*)data, length * 2, tout) != 0)
        return length;
    else
        return -1;
}

// uint8_t I2Cdev_readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length,
//     uint16_t* data, uint16_t timeout) {
//     uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;

//     HAL_I2C_Master_Transmit(&I2Cdev_hi2c, devAddr << 1, &regAddr, 1, tout);
//     if (HAL_I2C_Master_Receive(
//             &I2Cdev_hi2c, devAddr << 1, (uint8_t*)data, length * 2, tout)
//         == HAL_OK)
//         return length;
//     else
//         return -1;
// }

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBit(
    uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    I2Cdev_readByte(devAddr, regAddr, &b, I2Cdev_readTimeout);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return I2Cdev_writeByte(devAddr, regAddr, b);
}

/** write a single bit in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-15)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBitW(
    uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data) {
    uint16_t w;
    I2Cdev_readWord(devAddr, regAddr, &w, 100);
    w = (data != 0) ? (w | (1 << bitNum)) : (w & ~(1 << bitNum));
    return I2Cdev_writeWord(devAddr, regAddr, w);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b;
    if (I2Cdev_readByte(devAddr, regAddr, &b, 100) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return I2Cdev_writeByte(devAddr, regAddr, b);
    } else {
        return 0;
    }
}

/** Write multiple bits in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-15)
 * @param length Number of bits to write (not more than 16)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart,
    uint8_t length, uint16_t data) {
    //              010 value to write
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    // 0001110000000000 mask word
    // 1010111110010110 original value (sample)
    // 1010001110010110 original & ~mask
    // 1010101110010110 masked | value
    uint16_t w;
    if (I2Cdev_readWord(devAddr, regAddr, &w, 100) != 0) {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        w &= ~(mask); // zero all important bits in existing word
        w |= data; // combine data with existing word
        return I2Cdev_writeWord(devAddr, regAddr, w);
    } else {
        return 0;
    }
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    return I2Cdev_writeBytes(devAddr, regAddr, 1, &data);
}

/** Write single word to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data) {
    return I2Cdev_writeWords(devAddr, regAddr, 1, &data);
}

/** Write multiple bytes to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBytes(
    uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* pData) {
    // Creating array to store regAddr + data in one buffer
    uint8_t buffer[length + 1];
    buffer[0] = regAddr;

    // copy array
    memcpy(buffer + 1, pData, sizeof(uint8_t) * length);

    auto ok = I2Cdev_Master_Transmit(devAddr, buffer, length + 1, 1000);
    return ok;
}

/** Write multiple words to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of words to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeWords(
    uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t* data) {
    // Creating array to store regAddr + data in one buffer
    uint8_t buffer[length + 1];
    buffer[0] = regAddr;

    // copy array
    memcpy(buffer + 1, data, sizeof(uint16_t) * length);
    auto ok = I2Cdev_Master_Transmit(
        devAddr, buffer, sizeof(uint8_t) + sizeof(uint16_t) * length, 1000);
    return ok;
}
