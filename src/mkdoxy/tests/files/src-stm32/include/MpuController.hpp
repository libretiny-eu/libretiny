// I2Cdev library collection - mpu I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// 11/28/2014 by Marton Sebok <sebokmarton@gmail.com>
//
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     ...        - ongoing debug release
//     2014-11-28 - ported to PIC18 peripheral library from Arduino code
//     2017-03-11 - tested basic functions on STM32

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg
Copyright (c) 2014 Marton Sebok

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

#ifndef _mpu_H_
#define _mpu_H_

#include "I2cController.hpp"
#include "rbcx.pb.h"
#include <stdbool.h>

#if ((defined mpu_INCLUDE_DMP_MOTIONAPPS20)                                    \
    || (defined mpu_INCLUDE_DMP_MOTIONAPPS41))
#error DMP is not supported yet
#endif

#define mpu_ADDRESS_AD0_LOW                                                    \
    0x68 // address pin low (GND), default for InvenSense evaluation board
#define mpu_ADDRESS_AD0_HIGH 0x69 // address pin high (VCC)
#define mpu_DEFAULT_ADDRESS mpu_ADDRESS_AD0_LOW

#define mpu_RA_XG_OFFS_TC 0x00 //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define mpu_RA_YG_OFFS_TC 0x01 //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define mpu_RA_ZG_OFFS_TC 0x02 //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define mpu_RA_X_FINE_GAIN 0x03 //[7:0] X_FINE_GAIN
#define mpu_RA_Y_FINE_GAIN 0x04 //[7:0] Y_FINE_GAIN
#define mpu_RA_Z_FINE_GAIN 0x05 //[7:0] Z_FINE_GAIN
#define mpu_RA_XA_OFFS_H 0x06 //[15:0] XA_OFFS
#define mpu_RA_XA_OFFS_L_TC 0x07
#define mpu_RA_YA_OFFS_H 0x08 //[15:0] YA_OFFS
#define mpu_RA_YA_OFFS_L_TC 0x09
#define mpu_RA_ZA_OFFS_H 0x0A //[15:0] ZA_OFFS
#define mpu_RA_ZA_OFFS_L_TC 0x0B
#define mpu_RA_XG_OFFS_USRH 0x13 //[15:0] XG_OFFS_USR
#define mpu_RA_XG_OFFS_USRL 0x14
#define mpu_RA_YG_OFFS_USRH 0x15 //[15:0] YG_OFFS_USR
#define mpu_RA_YG_OFFS_USRL 0x16
#define mpu_RA_ZG_OFFS_USRH 0x17 //[15:0] ZG_OFFS_USR
#define mpu_RA_ZG_OFFS_USRL 0x18
#define mpu_RA_SMPLRT_DIV 0x19
#define mpu_RA_CONFIG 0x1A
#define mpu_RA_GYRO_CONFIG 0x1B
#define mpu_RA_ACCEL_CONFIG 0x1C
#define mpu_RA_FF_THR 0x1D
#define mpu_RA_FF_DUR 0x1E
#define mpu_RA_MOT_THR 0x1F
#define mpu_RA_MOT_DUR 0x20
#define mpu_RA_ZRMOT_THR 0x21
#define mpu_RA_ZRMOT_DUR 0x22
#define mpu_RA_FIFO_EN 0x23
#define mpu_RA_I2C_MST_CTRL 0x24
#define mpu_RA_I2C_SLV0_ADDR 0x25
#define mpu_RA_I2C_SLV0_REG 0x26
#define mpu_RA_I2C_SLV0_CTRL 0x27
#define mpu_RA_I2C_SLV1_ADDR 0x28
#define mpu_RA_I2C_SLV1_REG 0x29
#define mpu_RA_I2C_SLV1_CTRL 0x2A
#define mpu_RA_I2C_SLV2_ADDR 0x2B
#define mpu_RA_I2C_SLV2_REG 0x2C
#define mpu_RA_I2C_SLV2_CTRL 0x2D
#define mpu_RA_I2C_SLV3_ADDR 0x2E
#define mpu_RA_I2C_SLV3_REG 0x2F
#define mpu_RA_I2C_SLV3_CTRL 0x30
#define mpu_RA_I2C_SLV4_ADDR 0x31
#define mpu_RA_I2C_SLV4_REG 0x32
#define mpu_RA_I2C_SLV4_DO 0x33
#define mpu_RA_I2C_SLV4_CTRL 0x34
#define mpu_RA_I2C_SLV4_DI 0x35
#define mpu_RA_I2C_MST_STATUS 0x36
#define mpu_RA_INT_PIN_CFG 0x37
#define mpu_RA_INT_ENABLE 0x38
#define mpu_RA_DMP_INT_STATUS 0x39
#define mpu_RA_INT_STATUS 0x3A
#define mpu_RA_ACCEL_XOUT_H 0x3B
#define mpu_RA_ACCEL_XOUT_L 0x3C
#define mpu_RA_ACCEL_YOUT_H 0x3D
#define mpu_RA_ACCEL_YOUT_L 0x3E
#define mpu_RA_ACCEL_ZOUT_H 0x3F
#define mpu_RA_ACCEL_ZOUT_L 0x40
#define mpu_RA_TEMP_OUT_H 0x41
#define mpu_RA_TEMP_OUT_L 0x42
#define mpu_RA_GYRO_XOUT_H 0x43
#define mpu_RA_GYRO_XOUT_L 0x44
#define mpu_RA_GYRO_YOUT_H 0x45
#define mpu_RA_GYRO_YOUT_L 0x46
#define mpu_RA_GYRO_ZOUT_H 0x47
#define mpu_RA_GYRO_ZOUT_L 0x48
#define mpu_RA_EXT_SENS_DATA_00 0x49
#define mpu_RA_EXT_SENS_DATA_01 0x4A
#define mpu_RA_EXT_SENS_DATA_02 0x4B
#define mpu_RA_EXT_SENS_DATA_03 0x4C
#define mpu_RA_EXT_SENS_DATA_04 0x4D
#define mpu_RA_EXT_SENS_DATA_05 0x4E
#define mpu_RA_EXT_SENS_DATA_06 0x4F
#define mpu_RA_EXT_SENS_DATA_07 0x50
#define mpu_RA_EXT_SENS_DATA_08 0x51
#define mpu_RA_EXT_SENS_DATA_09 0x52
#define mpu_RA_EXT_SENS_DATA_10 0x53
#define mpu_RA_EXT_SENS_DATA_11 0x54
#define mpu_RA_EXT_SENS_DATA_12 0x55
#define mpu_RA_EXT_SENS_DATA_13 0x56
#define mpu_RA_EXT_SENS_DATA_14 0x57
#define mpu_RA_EXT_SENS_DATA_15 0x58
#define mpu_RA_EXT_SENS_DATA_16 0x59
#define mpu_RA_EXT_SENS_DATA_17 0x5A
#define mpu_RA_EXT_SENS_DATA_18 0x5B
#define mpu_RA_EXT_SENS_DATA_19 0x5C
#define mpu_RA_EXT_SENS_DATA_20 0x5D
#define mpu_RA_EXT_SENS_DATA_21 0x5E
#define mpu_RA_EXT_SENS_DATA_22 0x5F
#define mpu_RA_EXT_SENS_DATA_23 0x60
#define mpu_RA_MOT_DETECT_STATUS 0x61
#define mpu_RA_I2C_SLV0_DO 0x63
#define mpu_RA_I2C_SLV1_DO 0x64
#define mpu_RA_I2C_SLV2_DO 0x65
#define mpu_RA_I2C_SLV3_DO 0x66
#define mpu_RA_I2C_MST_DELAY_CTRL 0x67
#define mpu_RA_SIGNAL_PATH_RESET 0x68
#define mpu_RA_MOT_DETECT_CTRL 0x69
#define mpu_RA_USER_CTRL 0x6A
#define mpu_RA_PWR_MGMT_1 0x6B
#define mpu_RA_PWR_MGMT_2 0x6C
#define mpu_RA_BANK_SEL 0x6D
#define mpu_RA_MEM_START_ADDR 0x6E
#define mpu_RA_MEM_R_W 0x6F
#define mpu_RA_DMP_CFG_1 0x70
#define mpu_RA_DMP_CFG_2 0x71
#define mpu_RA_FIFO_COUNTH 0x72
#define mpu_RA_FIFO_COUNTL 0x73
#define mpu_RA_FIFO_R_W 0x74
#define mpu_RA_WHO_AM_I 0x75

#define mpu_TC_PWR_MODE_BIT 7
#define mpu_TC_OFFSET_BIT 6
#define mpu_TC_OFFSET_LENGTH 6
#define mpu_TC_OTP_BNK_VLD_BIT 0

#define mpu_VDDIO_LEVEL_VLOGIC 0
#define mpu_VDDIO_LEVEL_VDD 1

#define mpu_CFG_EXT_SYNC_SET_BIT 5
#define mpu_CFG_EXT_SYNC_SET_LENGTH 3
#define mpu_CFG_DLPF_CFG_BIT 2
#define mpu_CFG_DLPF_CFG_LENGTH 3

#define mpu_EXT_SYNC_DISABLED 0x0
#define mpu_EXT_SYNC_TEMP_OUT_L 0x1
#define mpu_EXT_SYNC_GYRO_XOUT_L 0x2
#define mpu_EXT_SYNC_GYRO_YOUT_L 0x3
#define mpu_EXT_SYNC_GYRO_ZOUT_L 0x4
#define mpu_EXT_SYNC_ACCEL_XOUT_L 0x5
#define mpu_EXT_SYNC_ACCEL_YOUT_L 0x6
#define mpu_EXT_SYNC_ACCEL_ZOUT_L 0x7

#define mpu_DLPF_BW_256 0x00
#define mpu_DLPF_BW_188 0x01
#define mpu_DLPF_BW_98 0x02
#define mpu_DLPF_BW_42 0x03
#define mpu_DLPF_BW_20 0x04
#define mpu_DLPF_BW_10 0x05
#define mpu_DLPF_BW_5 0x06

#define mpu_GCONFIG_FS_SEL_BIT 4
#define mpu_GCONFIG_FS_SEL_LENGTH 2

#define mpu_GYRO_FS_250 0x00
#define mpu_GYRO_FS_500 0x01
#define mpu_GYRO_FS_1000 0x02
#define mpu_GYRO_FS_2000 0x03

#define mpu_ACONFIG_XA_ST_BIT 7
#define mpu_ACONFIG_YA_ST_BIT 6
#define mpu_ACONFIG_ZA_ST_BIT 5
#define mpu_ACONFIG_AFS_SEL_BIT 4
#define mpu_ACONFIG_AFS_SEL_LENGTH 2
#define mpu_ACONFIG_ACCEL_HPF_BIT 2
#define mpu_ACONFIG_ACCEL_HPF_LENGTH 3

#define mpu_ACCEL_FS_2 0x00
#define mpu_ACCEL_FS_4 0x01
#define mpu_ACCEL_FS_8 0x02
#define mpu_ACCEL_FS_16 0x03

#define mpu_DHPF_RESET 0x00
#define mpu_DHPF_5 0x01
#define mpu_DHPF_2P5 0x02
#define mpu_DHPF_1P25 0x03
#define mpu_DHPF_0P63 0x04
#define mpu_DHPF_HOLD 0x07

#define mpu_TEMP_FIFO_EN_BIT 7
#define mpu_XG_FIFO_EN_BIT 6
#define mpu_YG_FIFO_EN_BIT 5
#define mpu_ZG_FIFO_EN_BIT 4
#define mpu_ACCEL_FIFO_EN_BIT 3
#define mpu_SLV2_FIFO_EN_BIT 2
#define mpu_SLV1_FIFO_EN_BIT 1
#define mpu_SLV0_FIFO_EN_BIT 0

#define mpu_MULT_MST_EN_BIT 7
#define mpu_WAIT_FOR_ES_BIT 6
#define mpu_SLV_3_FIFO_EN_BIT 5
#define mpu_I2C_MST_P_NSR_BIT 4
#define mpu_I2C_MST_CLK_BIT 3
#define mpu_I2C_MST_CLK_LENGTH 4

#define mpu_CLOCK_DIV_348 0x0
#define mpu_CLOCK_DIV_333 0x1
#define mpu_CLOCK_DIV_320 0x2
#define mpu_CLOCK_DIV_308 0x3
#define mpu_CLOCK_DIV_296 0x4
#define mpu_CLOCK_DIV_286 0x5
#define mpu_CLOCK_DIV_276 0x6
#define mpu_CLOCK_DIV_267 0x7
#define mpu_CLOCK_DIV_258 0x8
#define mpu_CLOCK_DIV_500 0x9
#define mpu_CLOCK_DIV_471 0xA
#define mpu_CLOCK_DIV_444 0xB
#define mpu_CLOCK_DIV_421 0xC
#define mpu_CLOCK_DIV_400 0xD
#define mpu_CLOCK_DIV_381 0xE
#define mpu_CLOCK_DIV_364 0xF

#define mpu_I2C_SLV_RW_BIT 7
#define mpu_I2C_SLV_ADDR_BIT 6
#define mpu_I2C_SLV_ADDR_LENGTH 7
#define mpu_I2C_SLV_EN_BIT 7
#define mpu_I2C_SLV_BYTE_SW_BIT 6
#define mpu_I2C_SLV_REG_DIS_BIT 5
#define mpu_I2C_SLV_GRP_BIT 4
#define mpu_I2C_SLV_LEN_BIT 3
#define mpu_I2C_SLV_LEN_LENGTH 4

#define mpu_I2C_SLV4_RW_BIT 7
#define mpu_I2C_SLV4_ADDR_BIT 6
#define mpu_I2C_SLV4_ADDR_LENGTH 7
#define mpu_I2C_SLV4_EN_BIT 7
#define mpu_I2C_SLV4_INT_EN_BIT 6
#define mpu_I2C_SLV4_REG_DIS_BIT 5
#define mpu_I2C_SLV4_MST_DLY_BIT 4
#define mpu_I2C_SLV4_MST_DLY_LENGTH 5

#define mpu_MST_PASS_THROUGH_BIT 7
#define mpu_MST_I2C_SLV4_DONE_BIT 6
#define mpu_MST_I2C_LOST_ARB_BIT 5
#define mpu_MST_I2C_SLV4_NACK_BIT 4
#define mpu_MST_I2C_SLV3_NACK_BIT 3
#define mpu_MST_I2C_SLV2_NACK_BIT 2
#define mpu_MST_I2C_SLV1_NACK_BIT 1
#define mpu_MST_I2C_SLV0_NACK_BIT 0

#define mpu_INTCFG_INT_LEVEL_BIT 7
#define mpu_INTCFG_INT_OPEN_BIT 6
#define mpu_INTCFG_LATCH_INT_EN_BIT 5
#define mpu_INTCFG_INT_RD_CLEAR_BIT 4
#define mpu_INTCFG_FSYNC_INT_LEVEL_BIT 3
#define mpu_INTCFG_FSYNC_INT_EN_BIT 2
#define mpu_INTCFG_I2C_BYPASS_EN_BIT 1
#define mpu_INTCFG_CLKOUT_EN_BIT 0

#define mpu_INTMODE_ACTIVEHIGH 0x00
#define mpu_INTMODE_ACTIVELOW 0x01

#define mpu_INTDRV_PUSHPULL 0x00
#define mpu_INTDRV_OPENDRAIN 0x01

#define mpu_INTLATCH_50USPULSE 0x00
#define mpu_INTLATCH_WAITCLEAR 0x01

#define mpu_INTCLEAR_STATUSREAD 0x00
#define mpu_INTCLEAR_ANYREAD 0x01

#define mpu_INTERRUPT_FF_BIT 7
#define mpu_INTERRUPT_MOT_BIT 6
#define mpu_INTERRUPT_ZMOT_BIT 5
#define mpu_INTERRUPT_FIFO_OFLOW_BIT 4
#define mpu_INTERRUPT_I2C_MST_INT_BIT 3
#define mpu_INTERRUPT_PLL_RDY_INT_BIT 2
#define mpu_INTERRUPT_DMP_INT_BIT 1
#define mpu_INTERRUPT_DATA_RDY_BIT 0

// TODO: figure out what these actually do
// UMPL source code is not very obivous
#define mpu_DMPINT_5_BIT 5
#define mpu_DMPINT_4_BIT 4
#define mpu_DMPINT_3_BIT 3
#define mpu_DMPINT_2_BIT 2
#define mpu_DMPINT_1_BIT 1
#define mpu_DMPINT_0_BIT 0

#define mpu_MOTION_MOT_XNEG_BIT 7
#define mpu_MOTION_MOT_XPOS_BIT 6
#define mpu_MOTION_MOT_YNEG_BIT 5
#define mpu_MOTION_MOT_YPOS_BIT 4
#define mpu_MOTION_MOT_ZNEG_BIT 3
#define mpu_MOTION_MOT_ZPOS_BIT 2
#define mpu_MOTION_MOT_ZRMOT_BIT 0

#define mpu_DELAYCTRL_DELAY_ES_SHADOW_BIT 7
#define mpu_DELAYCTRL_I2C_SLV4_DLY_EN_BIT 4
#define mpu_DELAYCTRL_I2C_SLV3_DLY_EN_BIT 3
#define mpu_DELAYCTRL_I2C_SLV2_DLY_EN_BIT 2
#define mpu_DELAYCTRL_I2C_SLV1_DLY_EN_BIT 1
#define mpu_DELAYCTRL_I2C_SLV0_DLY_EN_BIT 0

#define mpu_PATHRESET_GYRO_RESET_BIT 2
#define mpu_PATHRESET_ACCEL_RESET_BIT 1
#define mpu_PATHRESET_TEMP_RESET_BIT 0

#define mpu_DETECT_ACCEL_ON_DELAY_BIT 5
#define mpu_DETECT_ACCEL_ON_DELAY_LENGTH 2
#define mpu_DETECT_FF_COUNT_BIT 3
#define mpu_DETECT_FF_COUNT_LENGTH 2
#define mpu_DETECT_MOT_COUNT_BIT 1
#define mpu_DETECT_MOT_COUNT_LENGTH 2

#define mpu_DETECT_DECREMENT_RESET 0x0
#define mpu_DETECT_DECREMENT_1 0x1
#define mpu_DETECT_DECREMENT_2 0x2
#define mpu_DETECT_DECREMENT_4 0x3

#define mpu_USERCTRL_DMP_EN_BIT 7
#define mpu_USERCTRL_FIFO_EN_BIT 6
#define mpu_USERCTRL_I2C_MST_EN_BIT 5
#define mpu_USERCTRL_I2C_IF_DIS_BIT 4
#define mpu_USERCTRL_DMP_RESET_BIT 3
#define mpu_USERCTRL_FIFO_RESET_BIT 2
#define mpu_USERCTRL_I2C_MST_RESET_BIT 1
#define mpu_USERCTRL_SIG_COND_RESET_BIT 0

#define mpu_PWR1_DEVICE_RESET_BIT 7
#define mpu_PWR1_SLEEP_BIT 6
#define mpu_PWR1_CYCLE_BIT 5
#define mpu_PWR1_TEMP_DIS_BIT 3
#define mpu_PWR1_CLKSEL_BIT 2
#define mpu_PWR1_CLKSEL_LENGTH 3

#define mpu_CLOCK_INTERNAL 0x00
#define mpu_CLOCK_PLL_XGYRO 0x01
#define mpu_CLOCK_PLL_YGYRO 0x02
#define mpu_CLOCK_PLL_ZGYRO 0x03
#define mpu_CLOCK_PLL_EXT32K 0x04
#define mpu_CLOCK_PLL_EXT19M 0x05
#define mpu_CLOCK_KEEP_RESET 0x07

#define mpu_PWR2_LP_WAKE_CTRL_BIT 7
#define mpu_PWR2_LP_WAKE_CTRL_LENGTH 2
#define mpu_PWR2_STBY_XA_BIT 5
#define mpu_PWR2_STBY_YA_BIT 4
#define mpu_PWR2_STBY_ZA_BIT 3
#define mpu_PWR2_STBY_XG_BIT 2
#define mpu_PWR2_STBY_YG_BIT 1
#define mpu_PWR2_STBY_ZG_BIT 0

#define mpu_WAKE_FREQ_1P25 0x0
#define mpu_WAKE_FREQ_2P5 0x1
#define mpu_WAKE_FREQ_5 0x2
#define mpu_WAKE_FREQ_10 0x3

#define mpu_BANKSEL_PRFTCH_EN_BIT 6
#define mpu_BANKSEL_CFG_USER_BANK_BIT 5
#define mpu_BANKSEL_MEM_SEL_BIT 4
#define mpu_BANKSEL_MEM_SEL_LENGTH 5

#define mpu_WHO_AM_I_BIT 6
#define mpu_WHO_AM_I_LENGTH 6

#define mpu_DMP_MEMORY_BANKS 8
#define mpu_DMP_MEMORY_BANK_SIZE 256
#define mpu_DMP_MEMORY_CHUNK_SIZE 16

// note: DMP code memory blocks defined at end of header file

// #define COMPRESS_COEF 4

typedef struct mpu_t {
    uint8_t devAddr;
    uint8_t buffer[14];
} mpu_t;

void mpuDispatch(const CoprocReq_MpuReq& request);
void mpuTick();

void mpuCreate();
void mpuInitialize();
void mpuReset();
bool mpu_testConnection();

// AUX_VDDIO register
uint8_t mpu_getAuxVDDIOLevel();
void mpu_setAuxVDDIOLevel(uint8_t level);

// SMPLRT_DIV register
uint8_t mpu_getRate();
void mpu_setRate(uint8_t rate);

// CONFIG register
uint8_t mpu_getExternalFrameSync();
void mpu_setExternalFrameSync(uint8_t sync);
uint8_t mpu_getDLPFMode();
void mpu_setDLPFMode(uint8_t bandwidth);

// GYRO_CONFIG register
uint8_t mpu_getFullScaleGyroRange();
void mpu_setFullScaleGyroRange(uint8_t range);

// ACCEL_CONFIG register
bool mpu_getAccelXSelfTest();
void mpu_setAccelXSelfTest(bool enabled);
bool mpu_getAccelYSelfTest();
void mpu_setAccelYSelfTest(bool enabled);
bool mpu_getAccelZSelfTest();
void mpu_setAccelZSelfTest(bool enabled);
uint8_t mpu_getFullScaleAccelRange();
void mpu_setFullScaleAccelRange(uint8_t range);
uint8_t mpu_getDHPFMode();
void mpu_setDHPFMode(uint8_t mode);

// FF_THR register
uint8_t mpu_getFreefallDetectionThreshold();
void mpu_setFreefallDetectionThreshold(uint8_t threshold);

// FF_DUR register
uint8_t mpu_getFreefallDetectionDuration();
void mpu_setFreefallDetectionDuration(uint8_t duration);

// MOT_THR register
uint8_t mpu_getMotionDetectionThreshold();
void mpu_setMotionDetectionThreshold(uint8_t threshold);

// MOT_DUR register
uint8_t mpu_getMotionDetectionDuration();
void mpu_setMotionDetectionDuration(uint8_t duration);

// ZRMOT_THR register
uint8_t mpu_getZeroMotionDetectionThreshold();
void mpu_setZeroMotionDetectionThreshold(uint8_t threshold);

// ZRMOT_DUR register
uint8_t mpu_getZeroMotionDetectionDuration();
void mpu_setZeroMotionDetectionDuration(uint8_t duration);

// FIFO_EN register
bool mpu_getTempFIFOEnabled();
void mpu_setTempFIFOEnabled(bool enabled);
bool mpu_getXGyroFIFOEnabled();
void mpu_setXGyroFIFOEnabled(bool enabled);
bool mpu_getYGyroFIFOEnabled();
void mpu_setYGyroFIFOEnabled(bool enabled);
bool mpu_getZGyroFIFOEnabled();
void mpu_setZGyroFIFOEnabled(bool enabled);
bool mpu_getAccelFIFOEnabled();
void mpu_setAccelFIFOEnabled(bool enabled);
bool mpu_getSlave2FIFOEnabled();
void mpu_setSlave2FIFOEnabled(bool enabled);
bool mpu_getSlave1FIFOEnabled();
void mpu_setSlave1FIFOEnabled(bool enabled);
bool mpu_getSlave0FIFOEnabled();
void mpu_setSlave0FIFOEnabled(bool enabled);

// I2C_MST_CTRL register
bool mpu_getMultiMasterEnabled();
void mpu_setMultiMasterEnabled(bool enabled);
bool mpu_getWaitForExternalSensorEnabled();
void mpu_setWaitForExternalSensorEnabled(bool enabled);
bool mpu_getSlave3FIFOEnabled();
void mpu_setSlave3FIFOEnabled(bool enabled);
bool mpu_getSlaveReadWriteTransitionEnabled();
void mpu_setSlaveReadWriteTransitionEnabled(bool enabled);
uint8_t mpu_getMasterClockSpeed();
void mpu_setMasterClockSpeed(uint8_t speed);

// I2C_SLV* registers (Slave 0-3)
uint8_t mpu_getSlaveAddress(uint8_t num);
void mpu_setSlaveAddress(uint8_t num, uint8_t address);
uint8_t mpu_getSlaveRegister(uint8_t num);
void mpu_setSlaveRegister(uint8_t num, uint8_t reg);
bool mpu_getSlaveEnabled(uint8_t num);
void mpu_setSlaveEnabled(uint8_t num, bool enabled);
bool mpu_getSlaveWordByteSwap(uint8_t num);
void mpu_setSlaveWordByteSwap(uint8_t num, bool enabled);
bool mpu_getSlaveWriteMode(uint8_t num);
void mpu_setSlaveWriteMode(uint8_t num, bool mode);
bool mpu_getSlaveWordGroupOffset(uint8_t num);
void mpu_setSlaveWordGroupOffset(uint8_t num, bool enabled);
uint8_t mpu_getSlaveDataLength(uint8_t num);
void mpu_setSlaveDataLength(uint8_t num, uint8_t length);

// I2C_SLV* registers (Slave 4)
uint8_t mpu_getSlave4Address();
void mpu_setSlave4Address(uint8_t address);
uint8_t mpu_getSlave4Register();
void mpu_setSlave4Register(uint8_t reg);
void mpu_setSlave4OutputByte(uint8_t data);
bool mpu_getSlave4Enabled();
void mpu_setSlave4Enabled(bool enabled);
bool mpu_getSlave4InterruptEnabled();
void mpu_setSlave4InterruptEnabled(bool enabled);
bool mpu_getSlave4WriteMode();
void mpu_setSlave4WriteMode(bool mode);
uint8_t mpu_getSlave4MasterDelay();
void mpu_setSlave4MasterDelay(uint8_t delay);
uint8_t mpu_getSlate4InputByte();

// I2C_MST_STATUS register
bool mpu_getPassthroughStatus();
bool mpu_getSlave4IsDone();
bool mpu_getLostArbitration();
bool mpu_getSlave4Nack();
bool mpu_getSlave3Nack();
bool mpu_getSlave2Nack();
bool mpu_getSlave1Nack();
bool mpu_getSlave0Nack();

// INT_PIN_CFG register
bool mpu_getInterruptMode();
void mpu_setInterruptMode(bool mode);
bool mpu_getInterruptDrive();
void mpu_setInterruptDrive(bool drive);
bool mpu_getInterruptLatch();
void mpu_setInterruptLatch(bool latch);
bool mpu_getInterruptLatchClear();
void mpu_setInterruptLatchClear(bool clear);
bool mpu_getFSyncInterruptLevel();
void mpu_setFSyncInterruptLevel(bool level);
bool mpu_getFSyncInterruptEnabled();
void mpu_setFSyncInterruptEnabled(bool enabled);
bool mpu_getI2CBypassEnabled();
void mpu_setI2CBypassEnabled(bool enabled);
bool mpu_getClockOutputEnabled();
void mpu_setClockOutputEnabled(bool enabled);

// INT_ENABLE register
uint8_t mpu_getIntEnabled();
void mpu_setIntEnabled(uint8_t enabled);
bool mpu_getIntFreefallEnabled();
void mpu_setIntFreefallEnabled(bool enabled);
bool mpu_getIntMotionEnabled();
void mpu_setIntMotionEnabled(bool enabled);
bool mpu_getIntZeroMotionEnabled();
void mpu_setIntZeroMotionEnabled(bool enabled);
bool mpu_getIntFIFOBufferOverflowEnabled();
void mpu_setIntFIFOBufferOverflowEnabled(bool enabled);
bool mpu_getIntI2CMasterEnabled();
void mpu_setIntI2CMasterEnabled(bool enabled);
bool mpu_getIntDataReadyEnabled();
void mpu_setIntDataReadyEnabled(bool enabled);

// INT_STATUS register
uint8_t mpu_getIntStatus();
bool mpu_getIntFreefallStatus();
bool mpu_getIntMotionStatus();
bool mpu_getIntZeroMotionStatus();
bool mpu_getIntFIFOBufferOverflowStatus();
bool mpu_getIntI2CMasterStatus();
bool mpu_getIntDataReadyStatus();

// ACCEL_*OUT_* registers
void mpu_getMotion9(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx,
    int16_t* gy, int16_t* gz, int16_t* mx, int16_t* my, int16_t* mz);
void mpu_getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx,
    int16_t* gy, int16_t* gz);
void mpu_getAcceleration(int16_t* x, int16_t* y, int16_t* z);
// void mpu_getAcceleration(int32_t* x, int32_t* y, int32_t* z);
int16_t mpu_getAccelerationX();
int16_t mpu_getAccelerationY();
int16_t mpu_getAccelerationZ();

// TEMP_OUT_* registers
int16_t mpu_getTemperature();

// GYRO_*OUT_* registers
void mpu_getRotation(int16_t* x, int16_t* y, int16_t* z);
// void mpu_getRotation(int32_t* x, int32_t* y, int32_t* z);
int16_t mpu_getRotationX();
int16_t mpu_getRotationY();
int16_t mpu_getRotationZ();

// EXT_SENS_DATA_* registers
uint8_t mpu_getExternalSensorByte(int position);
uint16_t mpu_getExternalSensorWord(int position);
uint32_t getExternalSensorDWord(int position);

// MOT_DETECT_STATUS register
bool mpu_getXNegMotionDetected();
bool mpu_getXPosMotionDetected();
bool mpu_getYNegMotionDetected();
bool mpu_getYPosMotionDetected();
bool mpu_getZNegMotionDetected();
bool mpu_getZPosMotionDetected();
bool mpu_getZeroMotionDetected();

// I2C_SLV*_DO register
void mpu_setSlaveOutputByte(uint8_t num, uint8_t data);

// I2C_MST_DELAY_CTRL register
bool mpu_getExternalShadowDelayEnabled();
void mpu_setExternalShadowDelayEnabled(bool enabled);
bool mpu_getSlaveDelayEnabled(uint8_t num);
void mpu_setSlaveDelayEnabled(uint8_t num, bool enabled);

// SIGNAL_PATH_RESET register
void mpu_resetGyroscopePath();
void mpu_resetAccelerometerPath();
void mpu_resetTemperaturePath();

// MOT_DETECT_CTRL register
uint8_t mpu_getAccelerometerPowerOnDelay();
void mpu_setAccelerometerPowerOnDelay(uint8_t delay);
uint8_t mpu_getFreefallDetectionCounterDecrement();
void mpu_setFreefallDetectionCounterDecrement(uint8_t decrement);
uint8_t mpu_getMotionDetectionCounterDecrement();
void mpu_setMotionDetectionCounterDecrement(uint8_t decrement);

// USER_CTRL register
bool mpu_getFIFOEnabled();
void mpu_setFIFOEnabled(bool enabled);
bool mpu_getI2CMasterModeEnabled();
void mpu_setI2CMasterModeEnabled(bool enabled);
void mpu_switchSPIEnabled(bool enabled);
void mpu_resetFIFO();
void mpu_resetI2CMaster();
void mpu_resetSensors();

// PWR_MGMT_1 register
void mpu_reset();
bool mpu_getSleepEnabled();
void mpu_setSleepEnabled(bool enabled);
bool mpu_getWakeCycleEnabled();
void mpu_setWakeCycleEnabled(bool enabled);
bool mpu_getTempSensorEnabled();
void mpu_setTempSensorEnabled(bool enabled);
uint8_t mpu_getClockSource();
void mpu_setClockSource(uint8_t source);

// PWR_MGMT_2 register
uint8_t mpu_getWakeFrequency();
void mpu_setWakeFrequency(uint8_t frequency);
bool mpu_getStandbyXAccelEnabled();
void mpu_setStandbyXAccelEnabled(bool enabled);
bool mpu_getStandbyYAccelEnabled();
void mpu_setStandbyYAccelEnabled(bool enabled);
bool mpu_getStandbyZAccelEnabled();
void mpu_setStandbyZAccelEnabled(bool enabled);
bool mpu_getStandbyXGyroEnabled();
void mpu_setStandbyXGyroEnabled(bool enabled);
bool mpu_getStandbyYGyroEnabled();
void mpu_setStandbyYGyroEnabled(bool enabled);
bool mpu_getStandbyZGyroEnabled();
void mpu_setStandbyZGyroEnabled(bool enabled);

// FIFO_COUNT_* registers
uint16_t mpu_getFIFOCount();

// FIFO_R_W register
uint8_t mpu_getFIFOByte();
void mpu_setFIFOByte(uint8_t data);
void mpu_getFIFOBytes(uint8_t* data, uint8_t length);

// WHO_AM_I register
uint8_t mpu_getDeviceID();
void mpu_setDeviceID(uint8_t id);

// ======== UNDOCUMENTED/DMP REGISTERS/METHODS ========

// XG_OFFS_TC register
uint8_t mpu_getOTPBankValid();
void mpu_setOTPBankValid(bool enabled);
int8_t getXGyroOffsetTC();
void mpu_setXGyroOffsetTC(int8_t offset);

// YG_OFFS_TC register
int8_t getYGyroOffsetTC();
void mpu_setYGyroOffsetTC(int8_t offset);

// ZG_OFFS_TC register
int8_t getZGyroOffsetTC();
void mpu_setZGyroOffsetTC(int8_t offset);

// X_FINE_GAIN register
int8_t getXFineGain();
void mpu_setXFineGain(int8_t gain);

// Y_FINE_GAIN register
int8_t getYFineGain();
void mpu_setYFineGain(int8_t gain);

// Z_FINE_GAIN register
int8_t getZFineGain();
void mpu_setZFineGain(int8_t gain);

// XA_OFFS_* registers
int16_t mpu_getXAccelOffset();
void mpu_setXAccelOffset(int16_t offset);

// YA_OFFS_* register
int16_t mpu_getYAccelOffset();
void mpu_setYAccelOffset(int16_t offset);

// ZA_OFFS_* register
int16_t mpu_getZAccelOffset();
void mpu_setZAccelOffset(int16_t offset);

// XG_OFFS_USR* registers
int16_t mpu_getXGyroOffset();
void mpu_setXGyroOffset(int16_t offset);

// YG_OFFS_USR* register
int16_t mpu_getYGyroOffset();
void mpu_setYGyroOffset(int16_t offset);

// ZG_OFFS_USR* register
int16_t mpu_getZGyroOffset();
void mpu_setZGyroOffset(int16_t offset);

// INT_ENABLE register (DMP functions)
bool mpu_getIntPLLReadyEnabled();
void mpu_setIntPLLReadyEnabled(bool enabled);
bool mpu_getIntDMPEnabled();
void mpu_setIntDMPEnabled(bool enabled);

// DMP_INT_STATUS
bool mpu_getDMPInt5Status();
bool mpu_getDMPInt4Status();
bool mpu_getDMPInt3Status();
bool mpu_getDMPInt2Status();
bool mpu_getDMPInt1Status();
bool mpu_getDMPInt0Status();

// INT_STATUS register (DMP functions)
bool mpu_getIntPLLReadyStatus();
bool mpu_getIntDMPStatus();

// USER_CTRL register (DMP functions)
bool mpu_getDMPEnabled();
void mpu_setDMPEnabled(bool enabled);
void mpu_resetDMP();

// BANK_SEL register
void mpu_setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank);

// MEM_START_ADDR register
void mpu_setMemoryStartAddress(uint8_t address);

// MEM_R_W register
uint8_t mpu_readMemoryByte();
void mpu_writeMemoryByte(uint8_t data);
void mpu_readMemoryBlock(
    uint8_t* data, uint16_t dataSize, uint8_t bank, uint8_t address);
//bool mpu_writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify, bool useProgMem);
//bool mpu_writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify);

//bool mpu_writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, bool useProgMem);
//bool mpu_writeProgDMPConfigurationSet(const uint8_t *data, uint16_t dataSize);

// DMP_CFG_1 register
uint8_t mpu_getDMPConfig1();
void mpu_setDMPConfig1(uint8_t config);

// DMP_CFG_2 register
uint8_t mpu_getDMPConfig2();
void mpu_setDMPConfig2(uint8_t config);

#endif /* _mpu_H_ */
