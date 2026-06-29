/// Implements USB CDC using libusb_stm

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "Bsp.hpp"
#include "CdcUartTunnel.hpp"
#include "Esp32Manager.hpp"
#include "UsbCdcLink.h"
#include "usb_cdc.h"
#include "utils/Debug.hpp"

enum {
    STRDESC_LANG,
    STRDESC_MANUFACTURER,
    STRDESC_PRODUCT,
    STRDESC_CDC_IFACE,
    STRDESC_DEBUG_IFACE,
    STRDESC_SERIAL,

    STRDESC_MAX,
};

enum {
    INTERFACE_TUNNEL_COMM,
    INTERFACE_TUNNEL_DATA,
    INTERFACE_DEBUG_COMM,
    INTERFACE_DEBUG_DATA,

    INTERFACE_COUNT_ALL,
    INTERFACE_COUNT_NODEBUG = INTERFACE_DEBUG_COMM,
};

extern "C" {
extern const struct usb_string_descriptor lang_desc;
extern const struct usb_string_descriptor manuf_desc_en;
extern const struct usb_string_descriptor prod_desc_en;
extern const struct usb_string_descriptor cdc_iface_desc_en;
extern const struct usb_string_descriptor debug_iface_desc_en;
extern struct usb_string_descriptor serial_number_desc_en;
};

static const struct usb_string_descriptor* const dtable[STRDESC_MAX] = {
    &lang_desc,
    &manuf_desc_en,
    &prod_desc_en,
    &cdc_iface_desc_en,
    &debug_iface_desc_en,
    &serial_number_desc_en,
};

#define DEBUG_DESCRIPTORS                                                      \
    struct usb_iad_descriptor debug_comm_iad;                                  \
    struct usb_interface_descriptor debug_comm;                                \
    struct usb_cdc_header_desc debug_cdc_hdr;                                  \
    struct usb_cdc_call_mgmt_desc debug_cdc_mgmt;                              \
    struct usb_cdc_acm_desc debug_cdc_acm;                                     \
    struct usb_cdc_union_desc debug_cdc_union;                                 \
    struct usb_endpoint_descriptor debug_comm_ep;                              \
    struct usb_interface_descriptor debug_data;                                \
    struct usb_endpoint_descriptor debug_data_eprx;                            \
    struct usb_endpoint_descriptor debug_data_eptx;

struct __debug_descriptors {
    DEBUG_DESCRIPTORS
} __attribute__((packed));

static constexpr size_t DebugDescriptorsSize = sizeof(__debug_descriptors);

struct cdc_config {
    struct usb_config_descriptor config;

    struct usb_iad_descriptor tunnel_comm_iad;
    struct usb_interface_descriptor tunnel_comm;
    struct usb_cdc_header_desc tunnel_cdc_hdr;
    struct usb_cdc_call_mgmt_desc tunnel_cdc_mgmt;
    struct usb_cdc_acm_desc tunnel_cdc_acm;
    struct usb_cdc_union_desc tunnel_cdc_union;
    struct usb_endpoint_descriptor tunnel_comm_ep;
    struct usb_interface_descriptor tunnel_data;
    struct usb_endpoint_descriptor tunnel_data_eprx;
    struct usb_endpoint_descriptor tunnel_data_eptx;

    DEBUG_DESCRIPTORS
} __attribute__((packed));

static const struct usb_device_descriptor device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_IAD,
    .bDeviceSubClass = USB_SUBCLASS_IAD,
    .bDeviceProtocol = USB_PROTO_IAD,
    .bMaxPacketSize0 = CDC_EP0_SIZE,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = STRDESC_MANUFACTURER,
    .iProduct = STRDESC_PRODUCT,
    .iSerialNumber = STRDESC_SERIAL,
    .bNumConfigurations = 1,
};

static struct cdc_config config_desc = {
    .config = {
        .bLength = sizeof(struct usb_config_descriptor),
        .bDescriptorType = USB_DTYPE_CONFIGURATION,
        .wTotalLength = sizeof(struct cdc_config),
        .bNumInterfaces = INTERFACE_COUNT_ALL,
        .bConfigurationValue = 1,
        .iConfiguration = NO_DESCRIPTOR,
        .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
        .bMaxPower = USB_CFG_POWER_MA(100),
    },

    .tunnel_comm_iad = {
        .bLength = sizeof(struct usb_iad_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
        .bFirstInterface = INTERFACE_TUNNEL_COMM,
        .bInterfaceCount = 2,
        .bFunctionClass = USB_CLASS_CDC,
        .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
        .bFunctionProtocol = USB_PROTO_NONE,
        .iFunction = NO_DESCRIPTOR,
    },
    .tunnel_comm = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = INTERFACE_TUNNEL_COMM,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_CLASS_CDC,
        .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
        .bInterfaceProtocol = USB_PROTO_NONE,
        .iInterface = NO_DESCRIPTOR,
    },
    .tunnel_cdc_hdr = {
        .bFunctionLength = sizeof(struct usb_cdc_header_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_HEADER,
        .bcdCDC = VERSION_BCD(1, 1, 0),
    },
    .tunnel_cdc_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_mgmt_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = INTERFACE_TUNNEL_DATA,
    },
    .tunnel_cdc_acm = {
        .bFunctionLength = sizeof(struct usb_cdc_acm_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_ACM,
        .bmCapabilities = 0,
    },
    .tunnel_cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_UNION,
        .bMasterInterface0 = INTERFACE_TUNNEL_COMM,
        .bSlaveInterface0 = INTERFACE_TUNNEL_DATA,
    },
    .tunnel_comm_ep = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_TUNNEL_NTF_EP,
        .bmAttributes = USB_EPTYPE_INTERRUPT,
        .wMaxPacketSize = CDC_NTF_SZ,
        .bInterval = 0xFF,
    },
    .tunnel_data = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = INTERFACE_TUNNEL_DATA,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_CLASS_CDC_DATA,
        .bInterfaceSubClass = USB_SUBCLASS_NONE,
        .bInterfaceProtocol = USB_PROTO_NONE,
        .iInterface = STRDESC_CDC_IFACE,
    },
    .tunnel_data_eprx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_TUNNEL_RXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },
    .tunnel_data_eptx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_TUNNEL_TXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },


    .debug_comm_iad = {
        .bLength = sizeof(struct usb_iad_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
        .bFirstInterface = INTERFACE_DEBUG_COMM,
        .bInterfaceCount = 2,
        .bFunctionClass = USB_CLASS_CDC,
        .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
        .bFunctionProtocol = USB_PROTO_NONE,
        .iFunction = NO_DESCRIPTOR,
    },
    .debug_comm = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = INTERFACE_DEBUG_COMM,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_CLASS_CDC,
        .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
        .bInterfaceProtocol = USB_PROTO_NONE,
        .iInterface = NO_DESCRIPTOR,
    },
    .debug_cdc_hdr = {
        .bFunctionLength = sizeof(struct usb_cdc_header_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_HEADER,
        .bcdCDC = VERSION_BCD(1, 1, 0),
    },
    .debug_cdc_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_mgmt_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = INTERFACE_DEBUG_DATA,
    },
    .debug_cdc_acm = {
        .bFunctionLength = sizeof(struct usb_cdc_acm_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_ACM,
        .bmCapabilities = 0,
    },
    .debug_cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_UNION,
        .bMasterInterface0 = INTERFACE_DEBUG_COMM,
        .bSlaveInterface0 = INTERFACE_DEBUG_DATA,
    },
    .debug_comm_ep = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_DEBUG_NTF_EP,
        .bmAttributes = USB_EPTYPE_INTERRUPT,
        .wMaxPacketSize = CDC_NTF_SZ,
        .bInterval = 0xFF,
    },
    .debug_data = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = INTERFACE_DEBUG_DATA,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_CLASS_CDC_DATA,
        .bInterfaceSubClass = USB_SUBCLASS_NONE,
        .bInterfaceProtocol = USB_PROTO_NONE,
        .iInterface = STRDESC_DEBUG_IFACE,
    },
    .debug_data_eprx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_DEBUG_RXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },
    .debug_data_eptx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_DEBUG_TXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },
};

usbd_device udev;
static uint32_t ubuf[0x20];
static bool enableDebugEp = false;

static struct usb_cdc_line_coding cdc_line_tunnel = {
    .dwDTERate = 115200,
    .bCharFormat = USB_CDC_1_STOP_BITS,
    .bParityType = USB_CDC_NO_PARITY,
    .bDataBits = 8,
};

static struct usb_cdc_line_coding cdc_line_debug = {
    .dwDTERate = 115200,
    .bCharFormat = USB_CDC_1_STOP_BITS,
    .bParityType = USB_CDC_NO_PARITY,
    .bDataBits = 8,
};

static usbd_respond cdc_getdesc(
    usbd_ctlreq* req, void** address, uint16_t* length) {
    const uint8_t dtype = req->wValue >> 8;
    const uint8_t dnumber = req->wValue & 0xFF;
    const void* desc;
    uint16_t len = 0;
    switch (dtype) {
    case USB_DTYPE_DEVICE:
        desc = &device_desc;
        break;
    case USB_DTYPE_CONFIGURATION:
        desc = &config_desc;
        len = config_desc.config.wTotalLength;
        break;
    case USB_DTYPE_STRING:
        if (dnumber < STRDESC_MAX) {
            desc = dtable[dnumber];
        } else {
            return usbd_fail;
        }
        break;
    default:
        return usbd_fail;
    }
    if (len == 0) {
        len = ((struct usb_header_descriptor*)desc)->bLength;
    }
    *address = (void*)desc;
    *length = len;
    return usbd_ack;
};

static void tunnel_check_for_dfu_request(
    const struct usb_cdc_line_coding* coding) {
#ifdef RBCX_SBOOT
    if (coding->dwDTERate == 12345 && coding->bParityType == USB_CDC_EVEN_PARITY
        && coding->bCharFormat == USB_CDC_2_STOP_BITS) {
        rebootToDfu();
    }
#endif
}

static usbd_respond cdc_control_tunnel(usbd_device* dev, usbd_ctlreq* req) {
    switch (req->bRequest) {
    case USB_CDC_SET_CONTROL_LINE_STATE: {
        const bool dtr = req->wValue & 0x01;
        const bool rts = req->wValue & 0x02;
        //DEBUG("CONTROL_LINE_STATE DTR %d RTS %d\n", (int)dtr, (int)rts);
        sEsp32Manager.onSerialBreakInIrq(dtr, rts);
        return usbd_ack;
    }
    case USB_CDC_SET_LINE_CODING: {
        if (req->wLength < sizeof(cdc_line_tunnel))
            return usbd_fail;

        auto* newCoding = (struct usb_cdc_line_coding*)req->data;

        tunnel_check_for_dfu_request(newCoding);

        if (!tunnelOnSetLineCodingInIrq(cdc_line_tunnel, *newCoding))
            return usbd_fail;

        memcpy(&cdc_line_tunnel, req->data, sizeof(cdc_line_tunnel));
        /*DEBUG("USB_CDC_SET_LINE_CODING %d %d %d %d\n",
            cdc_line_tunnel.dwDTERate, cdc_line_tunnel.bCharFormat,
            cdc_line_tunnel.bDataBits, cdc_line_tunnel.bParityType);*/
        return usbd_ack;
    }
    case USB_CDC_GET_LINE_CODING:
        dev->status.data_ptr = &cdc_line_tunnel;
        dev->status.data_count = sizeof(cdc_line_tunnel);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

static usbd_respond cdc_control_debug(usbd_device* dev, usbd_ctlreq* req) {
    switch (req->bRequest) {
    case USB_CDC_SET_CONTROL_LINE_STATE: {
        return usbd_ack;
    }
    case USB_CDC_SET_LINE_CODING: {
        if (req->wLength < sizeof(cdc_line_debug))
            return usbd_fail;
        memcpy(&cdc_line_debug, req->data, sizeof(cdc_line_debug));
        //DEBUG("USB_CDC_SET_LINE_CODING %d %d %d %d\n", cdc_line.dwDTERate,
        //  cdc_line.bCharFormat, cdc_line.bDataBits, cdc_line.bParityType);
        return usbd_ack;
    }
    case USB_CDC_GET_LINE_CODING:
        dev->status.data_ptr = &cdc_line_debug;
        dev->status.data_count = sizeof(cdc_line_debug);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

static usbd_respond cdc_control(
    usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType)
        == (USB_REQ_INTERFACE | USB_REQ_CLASS)) {
        switch (req->wIndex) {
        case INTERFACE_TUNNEL_COMM:
            return cdc_control_tunnel(dev, req);
        case INTERFACE_DEBUG_COMM:
            return cdc_control_debug(dev, req);
        }
    }
    return usbd_fail;
}

static usbd_respond cdc_setconf(usbd_device* dev, uint8_t cfg) {
    switch (cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, CDC_TUNNEL_NTF_EP);
        usbd_ep_deconfig(dev, CDC_TUNNEL_TXD_EP);
        usbd_ep_deconfig(dev, CDC_TUNNEL_RXD_EP);
        if (enableDebugEp) {
            usbd_ep_deconfig(dev, CDC_DEBUG_NTF_EP);
            usbd_ep_deconfig(dev, CDC_DEBUG_TXD_EP);
            usbd_ep_deconfig(dev, CDC_DEBUG_RXD_EP);
        }
        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, CDC_TUNNEL_RXD_EP,
            USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
        usbd_ep_config(dev, CDC_TUNNEL_TXD_EP,
            USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
        usbd_ep_config(
            dev, CDC_TUNNEL_NTF_EP, USB_EPTYPE_INTERRUPT, CDC_NTF_SZ);

        if (enableDebugEp) {
            usbd_ep_config(dev, CDC_DEBUG_RXD_EP,
                USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
            usbd_ep_config(dev, CDC_DEBUG_TXD_EP,
                USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
            usbd_ep_config(
                dev, CDC_DEBUG_NTF_EP, USB_EPTYPE_INTERRUPT, CDC_NTF_SZ);
        }
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

void cdcLinkInit() {
    __HAL_RCC_USB_CLK_ENABLE();

    // quickly charge Button capacitor
    pinInit(button3Pin, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH,
        true);
    pinWrite(button3Pin, 1);

    std::array<uint32_t, 3> uid;
    HAL_GetUID(uid.data());

    char buf[9];
    size_t sn_off = 0;
    for (auto u : uid) {
        snprintf(buf, sizeof(buf), "%08lx", u);
        for (int i = 0; i < 8; ++i) {
            serial_number_desc_en.wString[sn_off++] = buf[i];
        }
    }

    // reinit button and check
    for (volatile int i = 0; i < 5000; ++i)
        ;
    pinInit(
        button3Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_LOW, true);
    enableDebugEp = !pinRead(button3Pin)
        || (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk);

    if (enableDebugEp) {
        config_desc.config.bNumInterfaces = INTERFACE_COUNT_ALL;
        config_desc.config.wTotalLength = sizeof(config_desc);
    } else {
        config_desc.config.bNumInterfaces = INTERFACE_COUNT_NODEBUG;
        config_desc.config.wTotalLength
            = sizeof(config_desc) - DebugDescriptorsSize;
    }

    usbd_init(&udev, &usbd_hw, CDC_EP0_SIZE, ubuf, sizeof(ubuf));
    usbd_reg_config(&udev, cdc_setconf);
    usbd_reg_control(&udev, cdc_control);
    usbd_reg_descr(&udev, cdc_getdesc);

    HAL_NVIC_SetPriority(usbLpIRQn, usbLpIRQnPrio, 0);
    HAL_NVIC_EnableIRQ(usbLpIRQn);

    usbd_enable(&udev, true);
    usbd_connect(&udev, true);
}

bool cdcLinkIsDebugEpEnabled() { return enableDebugEp; }

extern "C" void USB_LP_IRQ_HANDLER(void) { usbd_poll(&udev); }
