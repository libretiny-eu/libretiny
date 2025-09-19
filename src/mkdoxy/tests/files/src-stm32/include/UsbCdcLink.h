#pragma once

#include "stm32.h"
#include "usb.h"

#define CDC_EP0_SIZE 0x08
#define CDC_TUNNEL_RXD_EP 0x01
#define CDC_TUNNEL_TXD_EP 0x81
#define CDC_TUNNEL_NTF_EP 0x82
#define CDC_DEBUG_RXD_EP 0x02
#define CDC_DEBUG_TXD_EP 0x83
#define CDC_DEBUG_NTF_EP 0x84

#define CDC_DATA_SZ 0x40
#define CDC_NTF_SZ 0x08

extern usbd_device udev;

#ifdef __cplusplus
extern "C" {
#endif

void cdcLinkInit();
bool cdcLinkIsDebugEpEnabled();

#ifdef __cplusplus
}
#endif
