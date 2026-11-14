/* Logical port 2 is the dedicated log UART, not the SDK's UART2. */
#if LT_ARD_HAS_SERIAL || DOXYGEN
#include "SerialPrivate.h"
extern "C" {
#include <ameba_soc.h>
#include <hal_log_uart.h>
/* Boot-crash instrumentation, see base/fixups/boot_trace.c (temporary). */
void lt_amb1_trace(const char *tag);
void lt_amb1_trace2(const char *tag, uint32_t value);
/* The app's own log backend (base/port/printf.c): direct polled log-UART write. */
void putchar_(char c);
void lt_amb1_log_puts(const char *s);
}

static void rxCallback(uint32_t id, SerialIrq event) {
	auto *data = reinterpret_cast<SerialData *>(id);
	if (event == RxIrq)
		while (serial_readable(&data->uart))
			data->buf->store_char(serial_getc(&data->uart));
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	lt_amb1_trace2("startup Serial.begin port", (uint32_t)port);
	if (!data)
		return;
	data->buf = rxBuf;
	if (port == 2) {
		/* Console RX is intentionally disabled until a dedicated IRQ owner
		 * replaces the ROM monitor; never steal its shared IRQ here. */
		data->initialized = true;
		return;
	}
	serial_init(&data->uart, (PinName)tx, (PinName)rx);
	data->initialized = true;
	serial_irq_handler(&data->uart, rxCallback, (uint32_t)data);
	if (rx != PIN_INVALID)
		serial_irq_set(&data->uart, RxIrq, 1);
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	lt_amb1_trace2("startup Serial.configure port", (uint32_t)port);
	if (!data || !data->initialized)
		return;
	if (port == 2) {
		if (config != SERIAL_8N1) {
			LT_E("Log UART supports 8N1 only");
			return;
		}
		/* The log UART is the ROM/loader console and the fault-report channel
		 * (38400 8N1, see PORTING.md). Re-programming its divisor from the app
		 * re-bauds every later ROM and SDK message, which is exactly the
		 * output the bench capture needs. Leave it on the loader's setting. */
#if 0
		LOGUART_SetBaud(baudrate);
#endif
		lt_amb1_trace("log uart left at loader baud");
	} else {
		SerialParity parity = ParityNone;
		if ((config & SERIAL_PARITY_MASK) == SERIAL_PARITY_EVEN)
			parity = ParityEven;
		else if ((config & SERIAL_PARITY_MASK) == SERIAL_PARITY_ODD)
			parity = ParityOdd;
		serial_baud(&data->uart, baudrate);
		serial_format(
			&data->uart,
			(config & SERIAL_DATA_MASK) == SERIAL_DATA_7 ? 7 : 8,
			parity,
			(config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2 ? 2 : 1
		);
	}
	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!data || !data->initialized)
		return;
	if (port != 2) {
		serial_irq_set(&data->uart, RxIrq, 0);
		serial_free(&data->uart);
	}
	data->initialized = false;
}

void SerialClass::flush() {
	if (!data || !data->initialized)
		return;
	if (port == 2) {
		while (!(HAL_READ32(0x40003000, 0x14) & (1 << 6))) {}
	} else {
		while (!serial_writable(&data->uart)) {}
	}
}

size_t SerialClass::write(uint8_t c) {
	/* Counters (printed through the ROM console path, so they cannot recurse
	 * into this function) answer two questions: is the app logging at all, and
	 * how many bytes has it pushed. First call and then one line per 4 KiB. */
	static uint32_t written;
	static uint32_t reported;
	if (!written) {
		lt_amb1_trace2("first Serial.write port", (uint32_t)port);
	}
	written++;
	if (written - reported >= 4096) {
		reported = written;
		lt_amb1_trace2("Serial bytes written", written);
	}
	if (!data || !data->initialized)
		return 0;
	if (port == 2)
		/* Was HalSerialPutcRtl8195a (gated SDK console backend + mbed driver on
		 * the same UART). Use the core's direct polled log-UART writer instead,
		 * the same backend the ROM banner/markers reach the H7 capture on. */
		putchar_((char)c);
	else
		serial_putc(&data->uart, c);
	return 1;
}
#endif
