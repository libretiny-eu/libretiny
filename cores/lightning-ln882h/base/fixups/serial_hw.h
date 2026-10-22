#ifndef __SERIAL_HW_H__
#define __SERIAL_HW_H__

#include "hal/hal_uart.h"
#include "ln_types.h"

struct SerialHardware;
struct Serial;

typedef enum {
    SER_PORT_UART0 = 0,
    SER_PORT_UART1 = 1,
    SER_PORT_UART2 = 2,
    SER_PORT_NUM   = 3,  /**< Number of serial ports */
    SER_PORT_ID_INVALID = SER_PORT_NUM
}serial_port_id_t;

struct SerialHardwareVT
{
    void   (*init       )(struct SerialHardware *ctx, struct Serial *ser, uint32_t baudrate);
    void   (*cleanup    )(struct SerialHardware *ctx);
    void   (*txStart    )(struct SerialHardware *ctx);
    int8_t (*txSending  )(struct SerialHardware *ctx);
    int8_t (*setBaudrate)(struct SerialHardware *ctx, uint32_t baudrate);
};

struct SerialHardware
{
    const struct SerialHardwareVT *table;
    unsigned char                 *txbuffer;
    unsigned char                 *rxbuffer;
    size_t                         txbuffer_size;
    size_t                         rxbuffer_size;
    void                          *hw_device;
    volatile int8_t                isSending;
};

struct SerialHardware *serial_hw_getdesc(serial_port_id_t port_id);



#endif /* __SERIAL_HW_H__ */
