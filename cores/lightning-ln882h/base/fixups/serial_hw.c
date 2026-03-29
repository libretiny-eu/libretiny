#include "proj_config.h"
#include "hal/hal_uart.h"
#include "serial/ln_serial.h"
#include "serial_hw.h"
#include "utils/debug/ln_assert.h"
#include "hal/hal_gpio.h"
#include "hal/hal_misc.h"

#include "reg_sysc_cmp.h"//gpio fullmux

#define UART0_TX_BUF_SIZE  CFG_UART0_TX_BUF_SIZE
#define UART0_RX_BUF_SIZE  CFG_UART0_RX_BUF_SIZE
#define UART1_TX_BUF_SIZE  CFG_UART1_TX_BUF_SIZE
#define UART1_RX_BUF_SIZE  CFG_UART1_RX_BUF_SIZE
#define UART2_TX_BUF_SIZE  CFG_UART2_TX_BUF_SIZE
#define UART2_RX_BUF_SIZE  CFG_UART2_RX_BUF_SIZE

/* TX and RX fifo buffer */
uint8_t uart0_txbuf[UART0_TX_BUF_SIZE];
uint8_t uart0_rxbuf[UART0_RX_BUF_SIZE];
uint8_t uart1_txbuf[UART1_TX_BUF_SIZE];
uint8_t uart1_rxbuf[UART1_RX_BUF_SIZE];
uint8_t uart2_txbuf[UART2_TX_BUF_SIZE];
uint8_t uart2_rxbuf[UART2_RX_BUF_SIZE];

/* From the high-level serial driver */
extern Serial_t serial_handles[SER_PORT_NUM];

/* UART device*/
typedef struct
{
    uint32_t     uart_base;
    uart_init_t_def  init_cfg;
} uart_dev_t;

static uart_dev_t g_uart0;
static uart_dev_t g_uart1;
static uart_dev_t g_uart2;

/* serial */
typedef struct
{
    struct SerialHardware  Hardware;
    struct Serial         *serial;
} ln_serial_t;
ln_serial_t uart_serial[SER_PORT_NUM];


static void uart_io_pin_request(struct Serial *serial)
{
    if (serial->port_id == SER_PORT_UART0)
    {
        hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_2,UART0_TX);
        hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_3,UART0_RX);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_2,HAL_ENABLE);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_3,HAL_ENABLE);
    }
    else if (serial->port_id == SER_PORT_UART1)
    {
        hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_8,UART1_RX);
        hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_9,UART1_TX);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_ENABLE);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_9,HAL_ENABLE);
    }
    else if (serial->port_id == SER_PORT_UART2)
    {
    }
}

static void uart_io_pin_release(struct Serial *serial)
{
    if (serial == NULL)
    {
        return;
    }

    if (serial->port_id == SER_PORT_UART0)
    {
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_2,HAL_DISABLE);
        hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_3,HAL_DISABLE);
    }
    else if (serial->port_id == SER_PORT_UART1)
    {
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_DISABLE);
        hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_9,HAL_DISABLE);
    }
    else if (serial->port_id == SER_PORT_UART2)
    {
    }
}

static void hw_uart0_init(struct SerialHardware *_hw, struct Serial *serial, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw && serial);
    hw = (ln_serial_t *)_hw;
    hw->serial = serial;

    g_uart0.uart_base          = UART0_BASE;
    g_uart0.init_cfg.baudrate  = baudrate;//115200 921600 2000000
    g_uart0.init_cfg.word_len  = UART_WORD_LEN_8;
    g_uart0.init_cfg.parity    = UART_PARITY_NONE;
    g_uart0.init_cfg.stop_bits = UART_STOP_BITS_1;
    g_uart0.init_cfg.over_sampl= UART_OVER_SAMPL_8;

    hal_uart_init(g_uart0.uart_base, &g_uart0.init_cfg);

    hal_uart_rx_mode_en(g_uart0.uart_base, HAL_ENABLE);
    hal_uart_tx_mode_en(g_uart0.uart_base, HAL_ENABLE);
    hal_uart_en(g_uart0.uart_base, HAL_ENABLE);

    hal_uart_it_en(g_uart0.uart_base, USART_IT_RXNE);
    //uart_it_enable(g_uart0.uart_base, USART_IT_TXE);//uart_it_enable(g_uart0.uart_base, USART_IT_TXE);

    NVIC_EnableIRQ(UART0_IRQn);

    //request pin for uart
    uart_io_pin_request(hw->serial);
}

static void hw_uart1_init(struct SerialHardware *_hw, struct Serial *serial, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw && serial);
    hw = (ln_serial_t *)_hw;
    hw->serial = serial;

    g_uart1.uart_base          = UART1_BASE;
    g_uart1.init_cfg.baudrate  = baudrate;//115200 921600 2000000
    g_uart1.init_cfg.word_len  = UART_WORD_LEN_8;
    g_uart1.init_cfg.parity    = UART_PARITY_NONE;
    g_uart1.init_cfg.stop_bits = UART_STOP_BITS_1;
    g_uart1.init_cfg.over_sampl= UART_OVER_SAMPL_8;

    hal_uart_init(g_uart1.uart_base, &g_uart1.init_cfg);

    hal_uart_rx_mode_en(g_uart1.uart_base, HAL_ENABLE);
    hal_uart_tx_mode_en(g_uart1.uart_base, HAL_ENABLE);
    hal_uart_en(g_uart1.uart_base, HAL_ENABLE);

    hal_uart_it_en(g_uart1.uart_base, USART_IT_RXNE);
    //uart_it_enable(g_uart1.uart_base, USART_IT_TXE);
    NVIC_EnableIRQ(UART1_IRQn);

    //request pin for uart
    uart_io_pin_request(hw->serial);
}

static void hw_uart2_init(struct SerialHardware *_hw, struct Serial *serial, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw && serial);
    hw = (ln_serial_t *)_hw;
    hw->serial = serial;

    g_uart2.uart_base          = UART2_BASE;
    g_uart2.init_cfg.baudrate  = baudrate;//115200 921600 2000000
    g_uart2.init_cfg.word_len  = UART_WORD_LEN_8;
    g_uart2.init_cfg.parity    = UART_PARITY_NONE;
    g_uart2.init_cfg.stop_bits = UART_STOP_BITS_1;
    g_uart2.init_cfg.over_sampl= UART_OVER_SAMPL_8;

    hal_uart_init(g_uart2.uart_base, &g_uart2.init_cfg);

    hal_uart_rx_mode_en(g_uart2.uart_base, HAL_ENABLE);
    hal_uart_tx_mode_en(g_uart2.uart_base, HAL_ENABLE);
    hal_uart_en(g_uart2.uart_base, HAL_ENABLE);

    hal_uart_it_en(g_uart2.uart_base, USART_IT_RXNE);
    //uart_it_enable(g_uart2.uart_base, USART_IT_TXE);
    NVIC_EnableIRQ(UART2_IRQn);

    //request pin for uart
    uart_io_pin_request(hw->serial);
}

static void hw_uart0_cleanup(struct SerialHardware *_hw)
{
    ln_serial_t *hw = NULL;
    LN_ASSERT(_hw);

    hal_misc_reset_uart0();
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_DisableIRQ(UART0_IRQn);

    hw = (ln_serial_t *)_hw;
    uart_io_pin_release(hw->serial);
    hw->serial = NULL;                                                          // must be reset to NULL
}

static void hw_uart1_cleanup(struct SerialHardware *_hw)
{
    ln_serial_t *hw = NULL;
    LN_ASSERT(_hw);

    hal_misc_reset_uart1();
    NVIC_ClearPendingIRQ(UART1_IRQn);
    NVIC_DisableIRQ(UART1_IRQn);

    hw = (ln_serial_t *)_hw;
    uart_io_pin_release(hw->serial);
    hw->serial = NULL;                                                          // must be reset to NULL
}

static void hw_uart2_cleanup(struct SerialHardware *_hw)
{
    ln_serial_t *hw = NULL;
    LN_ASSERT(_hw);

    hal_misc_reset_uart2();
    NVIC_ClearPendingIRQ(UART2_IRQn);
    NVIC_DisableIRQ(UART2_IRQn);

    hw = (ln_serial_t *)_hw;
    uart_io_pin_release(hw->serial);
    hw->serial = NULL;                                                          // must be reset to NULL
}

static void hw_uart_tx_start_polling(struct SerialHardware * _hw)
{
    uint8_t ch;
    ln_serial_t *hw = NULL;
    uart_dev_t * pdev;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;
    while(!fifo_isempty(&hw->serial->txfifo))
    {
        ch = fifo_pop(&hw->serial->txfifo);
        pdev = (uart_dev_t *)hw->Hardware.hw_device;
        while (hal_uart_flag_get(pdev->uart_base, USART_FLAG_TXE) != HAL_SET) {};
        //while (uart_flag_get(pdev->uart_base, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
        hal_uart_send_data(pdev->uart_base, ch);
    }
}

#if 0
static void hw_uart_tx_start_isr(struct SerialHardware * _hw)
{
    ln_serial_t *hw = NULL;
    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;

    if (hw->Hardware.isSending){
        return;
    }

    if(!fifo_isempty(&hw->serial->txfifo))
    {
        hw->Hardware.isSending = LN_TRUE;
        /* Enable TX empty interrupts. */
        uart_it_enable(UART0_BASE, USART_IT_TXE);
    }
}
#endif

static int8_t hw_uart_tx_is_sending(struct SerialHardware * _hw)
{
    ln_serial_t *hw = NULL;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;
    return hw->Hardware.isSending;
}

static int8_t hw_uart_set_baudrate(struct SerialHardware * _hw, uint32_t baudrate)
{
    ln_serial_t *hw = NULL;
    uart_dev_t * pdev;

    LN_ASSERT(_hw);
    hw = (ln_serial_t *)_hw;

    pdev = (uart_dev_t *)hw->Hardware.hw_device;

    hal_uart_baudrate_set(pdev->uart_base, baudrate);
    return  LN_TRUE;
}

/*
 * High-level interface data structures.
 */
static const struct SerialHardwareVT uart0_vtable =
{
    .init        = hw_uart0_init,
    .cleanup     = hw_uart0_cleanup,
    .txStart     = hw_uart_tx_start_polling,//hw_uart_tx_start_polling,//hw_uart_tx_start_isr
    .txSending   = hw_uart_tx_is_sending,
    .setBaudrate = hw_uart_set_baudrate,
};

static const struct SerialHardwareVT uart1_vtable =
{
    .init        = hw_uart1_init,
    .cleanup     = hw_uart1_cleanup,
    .txStart     = hw_uart_tx_start_polling,//hw_uart_tx_start_isr
    .txSending   = hw_uart_tx_is_sending,
    .setBaudrate = hw_uart_set_baudrate,
};

static const struct SerialHardwareVT uart2_vtable =
{
    .init        = hw_uart2_init,
    .cleanup     = hw_uart2_cleanup,
    .txStart     = hw_uart_tx_start_polling,//hw_uart_tx_start_isr
    .txSending   = hw_uart_tx_is_sending,
    .setBaudrate = hw_uart_set_baudrate,
};

ln_serial_t uart_serial[SER_PORT_NUM] =
{
    {
        .Hardware =
        {
          .table         = &uart0_vtable,
          .txbuffer      = uart0_txbuf,
          .rxbuffer      = uart0_rxbuf,
          .txbuffer_size = sizeof(uart0_txbuf),
          .rxbuffer_size = sizeof(uart0_rxbuf),
          .hw_device     = (void *)&g_uart0,
          .isSending     = LN_FALSE,
        },
        .serial = NULL,
    },
    {
        .Hardware =
        {
          .table         = &uart1_vtable,
          .txbuffer      = uart1_txbuf,
          .rxbuffer      = uart1_rxbuf,
          .txbuffer_size = sizeof(uart1_txbuf),
          .rxbuffer_size = sizeof(uart1_rxbuf),
          .hw_device     = (void *)&g_uart1,
          .isSending     = LN_FALSE,
        },
        .serial = NULL,
    },
    {
        .Hardware =
        {
          .table         = &uart2_vtable,
          .txbuffer      = uart2_txbuf,
          .rxbuffer      = uart2_rxbuf,
          .txbuffer_size = sizeof(uart2_txbuf),
          .rxbuffer_size = sizeof(uart2_rxbuf),
          .hw_device     = (void *)&g_uart2,
          .isSending     = LN_FALSE,
        },
        .serial = NULL,
    },
};

struct SerialHardware *serial_hw_getdesc(serial_port_id_t port_id)
{
    LN_ASSERT(port_id < SER_PORT_NUM);
    return  (struct SerialHardware *)&uart_serial[port_id].Hardware;
}

///=====================UART0/1/2 IQR Handle===============================///
static inline void uart0_send_data_isr(void)
{
    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART0];
    uint8_t tx_char = 0;

    if (fifo_isempty(&hw->serial->txfifo))
    {
        hal_uart_it_disable(UART0_BASE, USART_IT_TXE);
        hw->Hardware.isSending = LN_FALSE;
    }
    else
    {
        tx_char = fifo_pop(&hw->serial->txfifo);
        hal_uart_send_data(UART0_BASE, tx_char);
        while (hal_uart_flag_get(UART0_BASE, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
    }
}

static inline void uart0_recv_data_isr(void)
{
    uint8_t ch = 0;

    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART0];

    while (fifo_isfull(&hw->serial->rxfifo)){
        serial_purge_rx(hw->serial);
    }

    ch = hal_uart_recv_data(UART0_BASE);

    fifo_push(&hw->serial->rxfifo, ch);
    hw->serial->rx_callback();
}

static inline void serial_uart0_isr_callback(void)
{
    if (hal_uart_it_en_status_get(UART0_BASE, USART_IT_RXNE) && \
        hal_uart_flag_get(UART0_BASE, USART_FLAG_RXNE)) {
        uart0_recv_data_isr();
    }

    if (hal_uart_it_en_status_get(UART0_BASE, USART_IT_TXE) && \
        hal_uart_flag_get(UART0_BASE, USART_FLAG_TXE)) {
        uart0_send_data_isr();
    }
}

static inline void uart1_send_data_isr(void)
{
    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART1];
    uint8_t tx_char = 0;

    if (fifo_isempty(&hw->serial->txfifo))
    {
        hal_uart_it_disable(UART1_BASE, USART_IT_TXE);
        hw->Hardware.isSending = LN_FALSE;
    }
    else
    {
        tx_char = fifo_pop(&hw->serial->txfifo);
        hal_uart_send_data(UART1_BASE, tx_char);
        while (hal_uart_flag_get(UART1_BASE, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
    }
}

static inline void uart1_recv_data_isr(void)
{
    uint8_t ch = 0;

    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART1];

    while (fifo_isfull(&hw->serial->rxfifo)){
        serial_purge_rx(hw->serial);
    }

    ch = hal_uart_recv_data(UART1_BASE);

    fifo_push(&hw->serial->rxfifo, ch);
    hw->serial->rx_callback();
}

static inline void serial_uart1_isr_callback(void)
{
    if (hal_uart_it_en_status_get(UART1_BASE, USART_IT_RXNE) && \
        hal_uart_flag_get(UART1_BASE, USART_FLAG_RXNE)) {
        uart1_recv_data_isr();
    }

    if (hal_uart_it_en_status_get(UART1_BASE, USART_IT_TXE) && \
        hal_uart_flag_get(UART1_BASE, USART_FLAG_TXE)) {
        uart1_send_data_isr();
    }
}

static inline void uart2_send_data_isr(void)
{
    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART2];
    uint8_t tx_char = 0;

    if (fifo_isempty(&hw->serial->txfifo))
    {
        hal_uart_it_disable(UART2_BASE, USART_IT_TXE);
        hw->Hardware.isSending = LN_FALSE;
    }
    else
    {
        tx_char = fifo_pop(&hw->serial->txfifo);
        hal_uart_send_data(UART2_BASE, tx_char);
        while (hal_uart_flag_get(UART2_BASE, USART_FLAG_TX_FIFO_FULL) == HAL_SET) {};
    }
}

static inline void uart2_recv_data_isr(void)
{
    uint8_t ch = 0;

    ln_serial_t *hw = (ln_serial_t *)&uart_serial[SER_PORT_UART2];

    while (fifo_isfull(&hw->serial->rxfifo)){
        serial_purge_rx(hw->serial);
    }

    ch = hal_uart_recv_data(UART2_BASE);

    fifo_push(&hw->serial->rxfifo, ch);
    hw->serial->rx_callback();
}

static inline void serial_uart2_isr_callback(void)
{
    if (hal_uart_it_en_status_get(UART2_BASE, USART_IT_RXNE) && \
        hal_uart_flag_get(UART2_BASE, USART_FLAG_RXNE)) {
        uart2_recv_data_isr();
    }

    if (hal_uart_it_en_status_get(UART2_BASE, USART_IT_TXE) && \
        hal_uart_flag_get(UART2_BASE, USART_FLAG_TXE)) {
        uart2_send_data_isr();
    }
}

void UART0_IRQHandler(void)
{
    serial_uart0_isr_callback();
}

void UART1_IRQHandler(void)
{
    serial_uart1_isr_callback();
}

void UART2_IRQHandler(void)
{
    serial_uart2_isr_callback();
}

