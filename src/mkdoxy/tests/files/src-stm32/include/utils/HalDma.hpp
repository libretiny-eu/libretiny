
#include "stm32f1xx_hal_dma.h"

// HAL_DMA_PollForTransfer, despite its name, can't be used
// for polling. When it hits timeout (even the 0 one), HAL_DMA_STATE_READY is set
// into the State member, regardless of whether the transfer is actually done or not.
// So you can't poll for whether the transfer is done. With the Poll function. What the fuck.
//
// This version will never set the State to HAL_DMA_STATE_READY on Timeout,
// only on error or when it is really done.
//
// Countless hours were wasted because of this garbage.
//
// Copied from https://github.com/STMicroelectronics/STM32CubeF1/blob/441b2cbdc25aa50437a59c4bffe22b88e78942c9/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
inline HAL_StatusTypeDef HAL_DMA_PollForTransfer_Really(
    DMA_HandleTypeDef* hdma, uint32_t CompleteLevel, uint32_t Timeout) {
    uint32_t temp;
    uint32_t tickstart = 0U;

    if (HAL_DMA_STATE_BUSY != hdma->State) {
        /* no transfer ongoing */
        hdma->ErrorCode = HAL_DMA_ERROR_NO_XFER;
        __HAL_UNLOCK(hdma);
        return HAL_ERROR;
    }

    /* Polling mode not supported in circular mode */
    if (RESET != (hdma->Instance->CCR & DMA_CCR_CIRC)) {
        hdma->ErrorCode = HAL_DMA_ERROR_NOT_SUPPORTED;
        return HAL_ERROR;
    }

    /* Get the level transfer complete flag */
    if (CompleteLevel == HAL_DMA_FULL_TRANSFER) {
        /* Transfer Complete flag */
        temp = __HAL_DMA_GET_TC_FLAG_INDEX(hdma);
    } else {
        /* Half Transfer Complete flag */
        temp = __HAL_DMA_GET_HT_FLAG_INDEX(hdma);
    }

    /* Get tick */
    tickstart = HAL_GetTick();

    while (__HAL_DMA_GET_FLAG(hdma, temp) == RESET) {
        if ((__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma))
                != RESET)) {
            /* When a DMA transfer error occurs */
            /* A hardware clear of its EN bits is performed */
            /* Clear all flags */
            hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);

            /* Update error code */
            SET_BIT(hdma->ErrorCode, HAL_DMA_ERROR_TE);

            /* Change the DMA state */
            hdma->State = HAL_DMA_STATE_READY;

            /* Process Unlocked */
            __HAL_UNLOCK(hdma);

            return HAL_ERROR;
        }
        /* Check for the Timeout */
        if (Timeout != HAL_MAX_DELAY) {
            if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) > Timeout)) {
                /* Update error code */
                SET_BIT(hdma->ErrorCode, HAL_DMA_ERROR_TIMEOUT);

                /* Change the DMA state */
                // Disabled, because it makes no fucking sense.
                //hdma->State = HAL_DMA_STATE_READY;

                /* Process Unlocked */

                // Don't unlock until transfer done.
                //__HAL_UNLOCK(hdma);

                return HAL_ERROR;
            }
        }
    }

    if (CompleteLevel == HAL_DMA_FULL_TRANSFER) {
        /* Clear the transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));

        /* The selected Channelx EN bit is cleared (DMA is disabled and
    all transfers are complete) */
        hdma->State = HAL_DMA_STATE_READY;
    } else {
        /* Clear the half transfer complete flag */
        __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma));
    }

    /* Process unlocked */
    __HAL_UNLOCK(hdma);

    return HAL_OK;
}
