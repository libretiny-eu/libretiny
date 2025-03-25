#include <array>
#include <string.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_ll_usart.h"

#include "Bsp.hpp"
#include "ButtonController.hpp"
#include "BuzzerController.hpp"
#include "Dispatcher.hpp"
#include "I2cController.hpp"
#include "Mpu6050.hpp"
#include "OledController.hpp"
#include "MpuController.hpp"
#include "Power.hpp"
#include "UsbCdcLink.h"
#include "coproc_codec.h"
#include "coproc_link_parser.h"
#include "rbcx.pb.h"
#include "utils/BasePriorityRaiser.hpp"
#include "utils/ByteFifo.hpp"
#include "utils/Debug.hpp"
#include "utils/MessageBufferWrapper.hpp"
#include "utils/MutexWrapper.hpp"
#include "utils/QueueWrapper.hpp"
#include "utils/StreamBufferWrapper.hpp"
#include "utils/TickTimer.hpp"

#include "rbcx.pb.h"

static DMA_HandleTypeDef dmaTxHandle;
static DMA_HandleTypeDef dmaRxHandle;

static constexpr size_t MaxLineLength = 128;

static std::array<uint8_t, 256> txDmaBuf;
static StreamBufferWrapper<txDmaBuf.size() * 4> txUartStreamBuf;

static ByteFifo<MaxLineLength> uartRxFifo;
static MessageBufferWrapper<MaxLineLength + 4> rxLineBuffer;

static std::array<uint8_t, CDC_DATA_SZ> usbFrameBuf;
static ByteFifo<MaxLineLength> usbRxFifo;
static ByteFifo<MaxLineLength * 4> usbTxFifo;
static BasePriorityRaiser<usbLpIRQnPrio> usbIrqPrioRaise;

void debugUartInit() {
    txUartStreamBuf.create();
    rxLineBuffer.create();

    LL_USART_InitTypeDef init;
    LL_USART_StructInit(&init);
    init.BaudRate = 115200;
    init.DataWidth = LL_USART_DATAWIDTH_8B;
    init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    init.Parity = LL_USART_PARITY_NONE;
    init.StopBits = LL_USART_STOPBITS_1;
    init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    if (LL_USART_Init(debugUart, &init) != SUCCESS)
        abort();
    LL_USART_Enable(debugUart);

    // UART RX runs indefinitely in circular mode
    dmaRxHandle.Instance = debugUartRxDmaChannel;
    dmaRxHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dmaRxHandle.Init.Mode = DMA_CIRCULAR;
    dmaRxHandle.Init.MemInc = DMA_MINC_ENABLE;
    dmaRxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    dmaRxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dmaRxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dmaRxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&dmaRxHandle);
    HAL_DMA_Start(&dmaRxHandle, uintptr_t(&(debugUart->DR)),
        uintptr_t(uartRxFifo.data()), uartRxFifo.size());
    LL_USART_EnableDMAReq_RX(debugUart);

    HAL_NVIC_SetPriority(debugUartIRQn, debugUartIrqPrio, 0);
    HAL_NVIC_EnableIRQ(debugUartIRQn);
    LL_USART_EnableIT_IDLE(debugUart);

    // UART TX burst is started ad hoc each time
    dmaTxHandle.Instance = debugUartTxDmaChannel;
    dmaTxHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dmaTxHandle.Init.Mode = DMA_NORMAL;
    dmaTxHandle.Init.MemInc = DMA_MINC_ENABLE;
    dmaTxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    dmaTxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dmaTxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dmaTxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&dmaTxHandle) != HAL_OK)
        abort();
    HAL_NVIC_SetPriority(debugUartTxDmaIRQn, debugUartTxDmaIrqPrio, 0);
    HAL_NVIC_EnableIRQ(debugUartTxDmaIRQn);
    LL_USART_EnableDMAReq_TX(debugUart);

    pinInit(debugUartTxPin, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    pinInit(
        debugUartRxPin, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
}

ssize_t debugLinkTx(const uint8_t* data, size_t len) {
    if ((size_t)len > sizeof(txDmaBuf))
        return -1;

    size_t res = 0;

    if (isInInterrupt()) {
        BaseType_t woken = pdFALSE;
        const auto status = taskENTER_CRITICAL_FROM_ISR();
        res = txUartStreamBuf.write((uint8_t*)data, len, 0, &woken);
        usbTxFifo.writeSpan((uint8_t*)data, len);
        taskEXIT_CRITICAL_FROM_ISR(status);
        portYIELD_FROM_ISR(woken);
    } else {
        while (txUartStreamBuf.freeSpace() < len)
            vTaskDelay(0);
        taskENTER_CRITICAL();
        res = txUartStreamBuf.write((uint8_t*)data, len, 0);
        usbTxFifo.writeSpan((uint8_t*)data, len);
        taskEXIT_CRITICAL();
    }

    if (res == 0)
        return -1;

    HAL_NVIC_SetPendingIRQ(debugUartTxDmaIRQn);
    return res;
}

extern "C" int _write(int fd, char* data, int len) {
    return debugLinkTx((uint8_t*)data, len);
}

template <int Size> static BaseType_t processRxBuf(ByteFifo<Size>& fifo) {
    const auto fifoAvailable = std::min(MaxLineLength, fifo.available());

    char buf[MaxLineLength];
    fifo.peekSpan((uint8_t*)buf, fifoAvailable);

    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    size_t remaining = fifoAvailable;
    char* start = buf;
    while (remaining != 0) {
        char* end = (char*)memchr(start, '\n', remaining);
        if (!end)
            break;

        *end = '\0';
        ++end;

        const size_t len = end - start;

        if (!rxLineBuffer.push_back(
                (uint8_t*)start, len, 0, &pxHigherPriorityTaskWoken)) {
            printf("Not enough space in line buffer, try sending commands "
                   "slower.\n");
        }

        remaining -= len;
        fifo.notifyRead(len);
        start = end;
    }
    return pxHigherPriorityTaskWoken;
}

extern "C" void DEBUGUART_HANDLER(void) {
    int rxHead = uartRxFifo.size() - __HAL_DMA_GET_COUNTER(&dmaRxHandle);
    uartRxFifo.setHead(rxHead);
    LL_USART_ClearFlag_IDLE(debugUart);
    portYIELD_FROM_ISR(processRxBuf(uartRxFifo));
}

extern "C" void DEBUGUART_TX_DMA_HANDLER() {
    HAL_DMA_IRQHandler(&dmaTxHandle);

    if (dmaTxHandle.State == HAL_DMA_STATE_READY) {
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
        const auto len = txUartStreamBuf.read(
            txDmaBuf.data(), txDmaBuf.size(), 0, &pxHigherPriorityTaskWoken);

        if (len > 0) {
            HAL_DMA_Start_IT(&dmaTxHandle, uint32_t(txDmaBuf.data()),
                uint32_t(&debugUart->DR), len);
        }

        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
}

#define COMMAND(name, ...)                                                     \
    if (strncmp(cmd, name " ", sizeof(name)) == 0) {                           \
        cmd += sizeof(name);                                                   \
        __VA_ARGS__                                                            \
    }

static void debugLinkHandleCommand(const char* cmd) {
    COMMAND("diag", {
        printTaskInfo();
        return;
    });

    COMMAND("power", {
        COMMAND("calibrate", {
            CoprocReq req = {
                .which_payload = CoprocReq_calibratePower_tag,
            };

            if (sscanf(cmd, "%lu %lu %lu %lu",
                    &req.payload.calibratePower.vccMv,
                    &req.payload.calibratePower.battMidMv,
                    &req.payload.calibratePower.vRef33Mv,
                    &req.payload.calibratePower.temperatureC)
                != 4) {
                printf("Invalid parameters!\n");
                return;
            }

            dispatcherEnqueueRequest(req);
            return;
        });

        COMMAND("info", {
            printf("Power info: VCC %d mV bMid: %dmV VRef: %d mv Temperature "
                   "%d C\n",
                powerBatteryMv(), powerBatteryMidMv(), powerVrefMv(),
                powerTemperatureC());
            return;
        });
    });

    COMMAND("motors", {
        COMMAND("pid", {
            CoprocReq req = {
                .which_payload = CoprocReq_motorReq_tag,
            };
            req.payload.motorReq.which_motorCmd
                = CoprocReq_MotorReq_setVelocityRegCoefs_tag;
            auto& c = req.payload.motorReq.motorCmd.setVelocityRegCoefs;
            if (sscanf(cmd, "%lu %lu %lu", &c.p, &c.i, &c.d) != 3) {
                printf("Invalid parameters!\n");
                return;
            }

            for (int m : { 0, 1, 2, 3 }) {
                req.payload.motorReq.motorIndex = m;
                dispatcherEnqueueRequest(req);
            }
            return;
        });

        COMMAND("power", {
            CoprocReq req = {
                .which_payload = CoprocReq_motorReq_tag,
            };
            req.payload.motorReq.which_motorCmd
                = CoprocReq_MotorReq_setPower_tag;
            auto& c = req.payload.motorReq;
            if (sscanf(cmd, "%lu %ld", &c.motorIndex, &c.motorCmd.setPower)
                != 2) {
                printf("Invalid parameters!\n");
                return;
            }

            dispatcherEnqueueRequest(req);
            return;
        });

        COMMAND("velocity", {
            CoprocReq req = {
                .which_payload = CoprocReq_motorReq_tag,
            };
            req.payload.motorReq.which_motorCmd
                = CoprocReq_MotorReq_setVelocity_tag;
            auto& c = req.payload.motorReq;
            if (sscanf(cmd, "%lu %ld", &c.motorIndex, &c.motorCmd.setVelocity)
                != 2) {
                printf("Invalid parameters!\n");
                return;
            }

            dispatcherEnqueueRequest(req);
            return;
        });

        COMMAND("position", {
            CoprocReq req = {
                .which_payload = CoprocReq_motorReq_tag,
            };
            auto& c = req.payload.motorReq;
            c.which_motorCmd = CoprocReq_MotorReq_setPosition_tag;
            if (sscanf(cmd, "%lu %ld %ld", &c.motorIndex,
                    &c.motorCmd.setPosition.targetPosition,
                    &c.motorCmd.setPosition.runningVelocity)
                != 3) {
                printf("Invalid parameters!\n");
                return;
            }

            dispatcherEnqueueRequest(req);
            return;
        });
    });

    COMMAND("leds", {
        COMMAND("set", {
            uint32_t val = 0;
            if (sscanf(cmd, "%lu", &val) != 1) {
                printf("Invalid parameters!\n");
                return;
            }
            setLeds(val);
            return;
        });
    });

    COMMAND("buzzer", {
        buzzerSetState(!pinRead(buzzerPin));
        return;
    });

    COMMAND("buttons", {
        COMMAND("debug", {
            buttonControllerSetDebug(true);
            return;
        });
    });

    COMMAND("oled", {
        COMMAND("test", {
            printf("Oled test: %d\n", oledTestConnection());
            return;
        });

        COMMAND("fill", {
            COMMAND("white", {               
                oledFill(White);
                oledUpdateScreen();
                printf("OLED fill white\n");
                return;
            });
            COMMAND("black", {
                oledFill(Black);
                oledUpdateScreen();
                printf("OLED fill black\n");
                return;
            });
        });
        COMMAND("write", {
            oledSetCursor(0, 0);
            oledWriteString("AHOJ", Font_11x18, White);
            oledUpdateScreen();
            printf("OLED write\n");
            return;
        });
        COMMAND("init", {
            CoprocReq_OledInit req;
            req.width = 128;
            req.height = 64;
            req.rotate = true;
            req.inverseColor = false;
            oledInit(req);
            printf("OLED init\n");
            return;
        });
    });

    COMMAND("mpu", {
        COMMAND("init", {
            mpuInitialize();
            printf("MPU init\n");
            return;
        });
        COMMAND("test", {
            printf("MPU test: %d\n", mpu_testConnection());
            return;
        });
        COMMAND("temp", {
            printf("MPU temp: %d\n", mpu_getTemperature());
            return;
        });
        COMMAND("acc", {
            int16_t x, y, z;
            // int32_t x, y, z;
            mpu_getAcceleration(&x, &y, &z);
            printf("MPU acc: x:%d, y:%d, z:%d\n", x, y, z);
            return;
        });
        COMMAND("gyro", {
            int16_t x, y, z;
            // int32_t x, y, z;
            mpu_getRotation(&x, &y, &z);
            printf("MPU gyro: x:%d, y:%d, z:%d\n", x, y, z);
            return;
        });
    });

    COMMAND("i2c", {
        COMMAND("transmit", {
            unsigned DevAddress;
            uint8_t pData[10];
            unsigned Size;
            if (sscanf(cmd, "%u %u %u", &DevAddress, &pData[0], &Size) != 3) {
                printf("Invalid parameters!\n");
                return;
            }
            printf("I2C tran %d\n",
                I2Cdev_Master_Transmit(DevAddress, pData, Size, 0));
            return;
        });

        COMMAND("receive", {
            unsigned DevAddress;
            uint8_t pData[10];
            unsigned Size;
            if (sscanf(cmd, "%u %u", &DevAddress, &Size) != 2) {
                printf("Invalid parameters!\n");
                return;
            }
            printf("I2C rec %d; ret: %d\n",
                I2Cdev_Master_Receive(DevAddress, pData, Size, 0), pData[0]);
            return;
        });

        COMMAND("ping", {
            unsigned DevAddress;
            unsigned Trials;
            if (sscanf(cmd, "%u %u", &DevAddress, &Trials) != 2) {
                printf("Invalid parameters!\n");
                return;
            }
            printf("I2C ping %d\n",
                I2Cdev_IsDeviceReady(DevAddress, Trials) == HAL_OK);
            return;
        });

        COMMAND("scan", {
            printf("I2C scanner %d\n", I2Cdev_scan());
            return;
        });
    });

    printf("Invalid command.\n");
}

static void debugDownstreamHandler() {
    usbIrqPrioRaise.lock();
    const int transferred = usbd_ep_read(
        &udev, CDC_DEBUG_RXD_EP, usbFrameBuf.data(), usbFrameBuf.size());
    usbIrqPrioRaise.unlock();

    if (transferred > 0) {
        usbRxFifo.writeSpan(usbFrameBuf.data(), transferred);
        processRxBuf(usbRxFifo);
    }
}

static void debugUpstreamHandler() {
    portDISABLE_INTERRUPTS();
    const auto chunk = std::min(usbTxFifo.available(), usbFrameBuf.size());
    if (chunk != 0) {
        usbTxFifo.peekSpan(usbFrameBuf.data(), chunk);
        int transferred
            = usbd_ep_write(&udev, CDC_DEBUG_TXD_EP, usbFrameBuf.data(), chunk);
        if (transferred > 0) {
            usbTxFifo.notifyRead(transferred);
        }
    }
    portENABLE_INTERRUPTS();
}

void debugLinkPoll() {
    char buf[MaxLineLength];
    if (rxLineBuffer.pop_front((uint8_t*)buf, MaxLineLength, 0)) {
        debugLinkHandleCommand(buf);
    }

    if (cdcLinkIsDebugEpEnabled()) {
        debugDownstreamHandler();
        debugUpstreamHandler();
    }
}
