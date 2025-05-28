/// Implements passthrough between the "tunnel" UART and USB CDC.
/// "Tunnel" UART leads to ESP32 main UART.

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_usart.h"

#include "FreeRTOS.h"

#include "Bsp.hpp"
#include "CdcUartTunnel.hpp"
#include "UsbCdcLink.h"
#include "utils/BasePriorityRaiser.hpp"
#include "utils/ByteFifo.hpp"
#include "utils/HalDma.hpp"

static DMA_HandleTypeDef dmaRxHandle;
static DMA_HandleTypeDef dmaTxHandle;
static ByteFifo<512> rxFifo;
static std::array<uint8_t, CDC_DATA_SZ> txBuf;
static BasePriorityRaiser<usbLpIRQnPrio> usbIrqPrioRaise;

void tunnelUartInit() {
    LL_USART_InitTypeDef init;
    LL_USART_StructInit(&init);
    init.BaudRate = 115200;
    init.DataWidth = LL_USART_DATAWIDTH_8B;
    init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    init.Parity = LL_USART_PARITY_NONE;
    init.StopBits = LL_USART_STOPBITS_1;
    init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    LL_USART_Init(tunnelUart, &init);
    LL_USART_Enable(tunnelUart);

    // UART RX runs indefinitely in circular mode
    dmaRxHandle.Instance = tunnelUartRxDmaChannel;
    dmaRxHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dmaRxHandle.Init.Mode = DMA_CIRCULAR;
    dmaRxHandle.Init.MemInc = DMA_MINC_ENABLE;
    dmaRxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    dmaRxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dmaRxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dmaRxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&dmaRxHandle);
    HAL_DMA_Start(&dmaRxHandle, uintptr_t(&(tunnelUart->DR)),
        uintptr_t(rxFifo.data()), rxFifo.size());
    LL_USART_EnableDMAReq_RX(tunnelUart);

    // UART TX burst is started ad hoc each time
    dmaTxHandle.Instance = tunnelUartTxDmaChannel;
    dmaTxHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dmaTxHandle.Init.Mode = DMA_NORMAL;
    dmaTxHandle.Init.MemInc = DMA_MINC_ENABLE;
    dmaTxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    dmaTxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dmaTxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dmaTxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&dmaTxHandle);
    LL_USART_EnableDMAReq_TX(tunnelUart);

    pinInit(
        tunnelUartTxPin, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    pinInit(
        tunnelUartRxPin, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
}

static void tunnelUartRxPoll() {
    int rxHead = rxFifo.size() - __HAL_DMA_GET_COUNTER(&dmaRxHandle);
    rxFifo.setHead(rxHead);
}

static void tunnelUartTx(uint8_t* data, size_t len) {
    HAL_DMA_Start(
        &dmaTxHandle, uintptr_t(data), uintptr_t(&tunnelUart->DR), len);
}

static bool tunnelUartTxReady() {
    HAL_DMA_PollForTransfer_Really(&dmaTxHandle, HAL_DMA_FULL_TRANSFER, 0);
    return dmaTxHandle.State == HAL_DMA_STATE_READY;
}

static void tunnelDownstreamHandler() {
    if (tunnelUartTxReady()) {
        usbIrqPrioRaise.lock();
        const int transferred = usbd_ep_read(
            &udev, CDC_TUNNEL_RXD_EP, txBuf.data(), txBuf.size());
        usbIrqPrioRaise.unlock();

        if (transferred > 0) {
            tunnelUartTx(txBuf.data(), transferred);
        }
    }
}

static void tunnelUpstreamHandler() {
    tunnelUartRxPoll();
    auto readable = rxFifo.readableSpan();
    if (readable.second > 0) {
        usbIrqPrioRaise.lock();
        const int transferred = usbd_ep_write(&udev, CDC_TUNNEL_TXD_EP,
            readable.first, std::min(readable.second, size_t(CDC_DATA_SZ)));
        usbIrqPrioRaise.unlock();

        if (transferred > 0) {
            rxFifo.notifyRead(transferred);
        }
    }
}

void tunnelPoll() {
    tunnelUpstreamHandler();
    tunnelDownstreamHandler();
}

bool tunnelOnSetLineCodingInIrq(
    const usb_cdc_line_coding& old, const usb_cdc_line_coding& current) {
    if (old.dwDTERate != current.dwDTERate) {
        // From inside of LL_USART_Init, wtf is this not exported as some function?
        // https://github.com/STMicroelectronics/STM32CubeF1/blob/master/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usart.c
        if (current.dwDTERate > 4500000U) {
            return false;
        }

        uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
        LL_RCC_ClocksTypeDef rcc_clocks;
        LL_RCC_GetSystemClocksFreq(&rcc_clocks);
        if (tunnelUart == USART1) {
            periphclk = rcc_clocks.PCLK2_Frequency;
        } else if (tunnelUart == USART2) {
            periphclk = rcc_clocks.PCLK1_Frequency;
        } else if (tunnelUart == USART3) {
            periphclk = rcc_clocks.PCLK1_Frequency;
        } else if (tunnelUart == UART4) {
            periphclk = rcc_clocks.PCLK1_Frequency;
        } else if (tunnelUart == UART5) {
            periphclk = rcc_clocks.PCLK1_Frequency;
        } else {
            abort();
        }
        LL_USART_SetBaudRate(tunnelUart, periphclk, current.dwDTERate);
    }

    if (current.bDataBits != 8) {
        return false;
    }

    if (old.bCharFormat != current.bCharFormat) {
        switch (current.bCharFormat) {
        case USB_CDC_1_STOP_BITS:
            LL_USART_SetStopBitsLength(tunnelUart, LL_USART_STOPBITS_1);
            break;
        case USB_CDC_1_5_STOP_BITS:
            LL_USART_SetStopBitsLength(tunnelUart, LL_USART_STOPBITS_1_5);
            break;
        case USB_CDC_2_STOP_BITS:
            LL_USART_SetStopBitsLength(tunnelUart, LL_USART_STOPBITS_2);
            break;
        default:
            return false;
        }
    }

    if (old.bParityType != current.bParityType) {
        switch (current.bParityType) {
        case USB_CDC_NO_PARITY:
            LL_USART_SetParity(tunnelUart, LL_USART_PARITY_NONE);
            break;
        case USB_CDC_ODD_PARITY:
            LL_USART_SetParity(tunnelUart, LL_USART_PARITY_ODD);
            break;
        case USB_CDC_EVEN_PARITY:
            LL_USART_SetParity(tunnelUart, LL_USART_PARITY_EVEN);
            break;
        default:
            return false;
        }
    }
    return true;
}
