#include "utils/Debug.hpp"
#include "Bsp.hpp"
#include "UsbCdcLink.h"

void rebootToDfu() {
#ifdef RBCX_SBOOT
    __disable_irq();

    usbd_connect(&udev, false);

    volatile uint32_t* st = (uint32_t*)0x02000bffc; // __stack
    *st = 0x157F32D4; // DFU_BOOTKEY

    NVIC_SystemReset();
#else
    abort();
#endif
}
