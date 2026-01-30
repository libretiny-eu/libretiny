/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include <SerialPrivate.h>

#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN)
static uint8_t adrState = 0;
static uint8_t adrCmd[] = {LT_UART_ADR_PATTERN};

void SerialClass::adrParse(uint8_t c) {
	adrState = (adrState + 1) * (c == adrCmd[adrState]);
	if (adrState == sizeof(adrCmd) / sizeof(uint8_t)) {
		LT_I("Auto download mode: rebooting");
		LT.restartDownloadMode();
	}
}
#endif

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	if (!this->data) {
		this->data	= new SerialData();
		this->rxBuf = new SerialRingBuffer();
	}

	this->beginPrivate(baudrate, config);
	// if (this->baudrate != baudrate || this->config != config)
	this->configure(baudrate, config);
}

void SerialClass::end() {
	this->endPrivate();

	delete this->data;
	delete this->rxBuf;
	this->data	   = nullptr;
	this->rxBuf	   = nullptr;
	this->baudrate = 0;
}

int SerialClass::available() {
	return this->rxBuf ? this->rxBuf->available() : 0;
}

int SerialClass::peek() {
	return this->rxBuf ? this->rxBuf->peek() : -1;
}

int SerialClass::read() {
	return this->rxBuf ? this->rxBuf->read_char() : -1;
}

#if LT_HW_UART0
SerialClass Serial0(0, PIN_SERIAL0_RX, PIN_SERIAL0_TX);
#endif
#if LT_HW_UART1
SerialClass Serial1(1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
#endif
#if LT_HW_UART2
SerialClass Serial2(2, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
#endif

#endif
