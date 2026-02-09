/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"
#include "serial_api.h"
#include "serial_ex_api.h"

static void disable_syslog_on_port(const uint8_t port)
{
   static bool s_syslogger_enabled = true;
   if (s_syslogger_enabled && (port == 2))
   {
      sys_log_uart_off();
      s_syslogger_enabled = false;
   }
}

static void empty_irq_callback(uint32_t, int) {}

static SerialParity parity_from_config(const uint16_t config){
   switch (config & SERIAL_PARITY_MASK) {
      case SERIAL_PARITY_NONE:
         return ParityNone;
      case SERIAL_PARITY_ODD:
         return ParityOdd;
      case SERIAL_PARITY_EVEN:
         return ParityEven;
      default:
         return ParityNone;
   }
}

static uint8_t datawidth_from_config(const uint16_t config){
   return (config & SERIAL_DATA_MASK) == SERIAL_DATA_7 ? 7 : 8;
}

static uint8_t stopbits_from_config(const uint16_t config){
   return (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2 ? 2 : 1;
}


void SerialClass::irq_callback(uint32_t id, int event) {
   if (event != RxIrq)
      return;
   SerialClass* sc = reinterpret_cast<SerialClass*>(id);

   uint8_t c = serial_getc(sc->data->uart);
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL2_RX != PIN_INVALID
   // parse UART protocol commands on UART2
   if (sc->port == 2)
      SerialClass::adrParse(c);
#endif
   sc->data->buf->store_char(c);
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
   if (!this->data)
      return;

   hal_uart_en_ctrl (this->port, 1);

   this->data->buf = this->rxBuf;

   delete this->data->uart;
   this->data->uart = new serial_t;
   memset(this->data->uart, 0, sizeof(serial_t));

   // TODO handle PIN_INVALID
   serial_init(this->data->uart, static_cast<PinName>(this->tx), static_cast<PinName>(this->rx));
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
   if (!this->data)
      return;

   if (this->data->uart)
   {
      serial_baud(this->data->uart, baudrate);
      serial_format(this->data->uart,
                    datawidth_from_config(config),
                    parity_from_config(config),
                    stopbits_from_config(config));

      serial_irq_handler(this->data->uart, reinterpret_cast<uart_irq_handler>(irq_callback), reinterpret_cast<uint32_t>(this));
      serial_irq_set(this->data->uart, RxIrq, 1);
   }

   this->baudrate = baudrate;
   this->config   = config;
}

void SerialClass::endPrivate() {
   disable_syslog_on_port(this->port);
   if (lt_log_get_port() == this->port)
   {
      lt_log_disable();
   }

   if (!this->data)
      return;

   if (!this->data->uart)
      return;

   serial_irq_set(this->data->uart, RxIrq, 0);
   serial_irq_handler(this->data->uart, nullptr, 0);
   flush();
   serial_free(this->data->uart);

   hal_uart_en_ctrl (this->port, 0);

   delete this->data->uart;
   this->data->uart = nullptr;
   this->data->buf = nullptr;
}

void SerialClass::flush() {
   if ((!this->data) || (!this->data->uart))
      return;
   serial_clear_tx(this->data->uart);
}

size_t SerialClass::write(uint8_t c) {
   if ((!this->data) || (!this->data->uart))
      return 0;
   serial_putc(this->data->uart, c);
   return 1;
}

#endif
