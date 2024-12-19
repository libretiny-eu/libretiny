/* Copyright (c) Kuba Szczodrzyński 2023-09-23. */

#if LT_ARD_HAS_SPI || DOXYGEN

#include <SPIPrivate.h>

SPIClass::~SPIClass() {}

int SPIClass::getPortByPins(pin_size_t sck, pin_size_t miso, pin_size_t mosi) {
	if (sck == PIN_INVALID || miso == PIN_INVALID || mosi == PIN_INVALID)
		return -1;
#if defined(PINS_SPI0_SCK) && defined(PINS_SPI0_MISO) && defined(PINS_SPI0_MOSI)
	if (ltArrayContains(PINS_SPI0_SCK, sck) && ltArrayContains(PINS_SPI0_MISO, miso) &&
		ltArrayContains(PINS_SPI0_MOSI, mosi))
		return 0;
#endif
#if defined(PINS_SPI1_SCK) && defined(PINS_SPI1_MISO) && defined(PINS_SPI1_MOSI)
	if (ltArrayContains(PINS_SPI1_SCK, sck) && ltArrayContains(PINS_SPI1_MISO, miso) &&
		ltArrayContains(PINS_SPI1_MOSI, mosi))
		return 1;
#endif
#if defined(PINS_SPI2_SCK) && defined(PINS_SPI2_MISO) && defined(PINS_SPI2_MOSI)
	if (ltArrayContains(PINS_SPI2_SCK, sck) && ltArrayContains(PINS_SPI2_MISO, miso) &&
		ltArrayContains(PINS_SPI2_MOSI, mosi))
		return 2;
#endif
	return -1;
}

bool SPIClass::setPinsPrivate(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs) {
	// default to already set pins
	if (sck == PIN_INVALID)
		sck = this->sck;
	if (miso == PIN_INVALID)
		miso = this->miso;
	if (mosi == PIN_INVALID)
		mosi = this->mosi;

	if (cs != PIN_INVALID)
		this->cs = cs;

	if (sck == PIN_INVALID && miso == PIN_INVALID && mosi == PIN_INVALID) {
		// set pins by port number
#if defined(PINS_SPI0_SCK) && defined(PINS_SPI0_MISO) && defined(PINS_SPI0_MOSI)
		if (this->port == 0) {
			this->sck  = PINS_SPI0_SCK[0];
			this->miso = PINS_SPI0_MISO[0];
			this->mosi = PINS_SPI0_MOSI[0];
			return true;
		}
#endif
#if defined(PINS_SPI1_SCK) && defined(PINS_SPI1_MISO) && defined(PINS_SPI1_MOSI)
		if (this->port == 1) {
			this->sck  = PINS_SPI1_SCK[0];
			this->miso = PINS_SPI1_MISO[0];
			this->mosi = PINS_SPI1_MOSI[0];
			return true;
		}
#endif
#if defined(PINS_SPI2_SCK) && defined(PINS_SPI2_MISO) && defined(PINS_SPI2_MOSI)
		if (this->port == 2) {
			this->sck  = PINS_SPI2_SCK[0];
			this->miso = PINS_SPI2_MISO[0];
			this->mosi = PINS_SPI2_MOSI[0];
			return true;
		}
#endif
	}

	// set port number by specified pins
	int port = this->getPortByPins(sck, miso, mosi);
	if (port == -1)
		// no such port
		return false;
	if (this->fixedPort && port != this->port)
		// allow only same port for fixed port instances
		return false;
	this->port = port;
	this->sck  = sck;
	this->miso = miso;
	this->mosi = mosi;
	return true;
}

bool SPIClass::begin(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs) {
	if (!this->setPinsPrivate(sck, miso, mosi, cs))
		return false;
	return this->begin();
}

bool SPIClass::begin() {
	if (!this->setPinsPrivate(PIN_INVALID, PIN_INVALID, PIN_INVALID))
		return false;

	LT_DM(SPI, "Begin: sck=%d, miso=%d, mosi=%d, port=%d", this->sck, this->miso, this->mosi, this->port);

	if (!this->data) {
		this->data = new SPIData();
	}

	if (!this->beginPrivate())
		return false;
	this->setFrequency(settings._clock);
	this->setBitOrder(settings._bitOrder);
	this->setDataMode(settings._dataMode);

	return true;
}

bool SPIClass::end() {
	if (!this->endPrivate())
		return false;

	delete this->data;
	this->data = nullptr;
	return true;
}

bool SPIClass::setPins(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs) {
	if (!this->data)
		return this->setPinsPrivate(sck, miso, mosi, cs);
	return this->begin(sck, miso, mosi, cs);
}

__attribute__((weak)) void SPIClass::beginTransaction(SPISettings settings) {
	auto oldSettings = this->settings;

	if (settings._clock != oldSettings._clock)
		this->setFrequency(settings._clock);
	if (settings._bitOrder != oldSettings._bitOrder)
		this->setBitOrder(settings._bitOrder);
	if (settings._dataMode != oldSettings._dataMode)
		this->setDataMode(settings._dataMode);

	// this->inTransaction = true;
	if (useCs && this->cs != PIN_INVALID) {
		digitalWrite(useCs, LOW);
	}
}

__attribute__((weak)) void SPIClass::endTransaction() {
	// this->inTransaction = false;
	if (useCs && this->cs != PIN_INVALID) {
		digitalWrite(useCs, HIGH);
	}
}

__attribute__((weak)) void SPIClass::setHwCs(bool useCs) {
	this->useCs = useCs;
	if (this->cs != PIN_INVALID) {
		if (useCs)
			pinMode(this->cs, OUTPUT);
		else
			pinModeRemove(this->cs, PIN_GPIO);
	}
}

__attribute__((weak)) void SPIClass::setClockDivider(uint32_t clockDiv) {
	this->setFrequency(LT.getCpuFreq() / clockDiv);
}

__attribute__((weak)) uint32_t SPIClass::getClockDivider() {
	return LT.getCpuFreq() / this->settings._clock;
}

__attribute__((weak)) void SPIClass::setBitOrder(uint8_t bitOrder) {
	this->settings._bitOrder = bitOrder;
}

__attribute__((weak)) uint8_t SPIClass::transfer(uint8_t data) {
	this->txBuf = &data;
	this->txLen = 1;
	this->rxBuf = &data;
	this->rxLen = 1;
	this->commitTransaction();
	return data;
}

__attribute__((weak)) uint16_t SPIClass::transfer16(uint16_t data) {
	ByteUint16 data16;
	data16.val = data;
	if (this->settings._bitOrder == LSBFIRST) {
		data16.lsb = this->transfer(data16.lsb);
		data16.msb = this->transfer(data16.msb);
	} else {
		data16.msb = this->transfer(data16.msb);
		data16.lsb = this->transfer(data16.lsb);
	}
	return data16.val;
}

__attribute__((weak)) uint32_t SPIClass::transfer32(uint32_t data) {
	ByteUint32 data32;
	data32.val = data;
	if (this->settings._bitOrder == LSBFIRST) {
		data32.lsb	 = this->transfer(data32.lsb);
		data32.byte1 = this->transfer(data32.byte1);
		data32.byte2 = this->transfer(data32.byte2);
		data32.msb	 = this->transfer(data32.msb);
	} else {
		data32.msb	 = this->transfer(data32.msb);
		data32.byte2 = this->transfer(data32.byte2);
		data32.byte1 = this->transfer(data32.byte1);
		data32.lsb	 = this->transfer(data32.lsb);
	}
	return data32.val;
}

__attribute__((weak)) void SPIClass::write(uint8_t data) {
	this->txBuf = &data;
	this->txLen = 1;
	this->rxLen = 0;
	this->commitTransaction();
}

__attribute__((weak)) void SPIClass::write16(uint16_t data) {
	ByteUint16 data16;
	data16.val = data;
	if (this->settings._bitOrder == LSBFIRST) {
		this->write(data16.lsb);
		this->write(data16.msb);
	} else {
		this->write(data16.msb);
		this->write(data16.lsb);
	}
}

__attribute__((weak)) void SPIClass::write32(uint32_t data) {
	ByteUint32 data32;
	data32.val = data;
	if (this->settings._bitOrder == LSBFIRST) {
		this->write(data32.lsb);
		this->write(data32.byte1);
		this->write(data32.byte2);
		this->write(data32.msb);
	} else {
		this->write(data32.msb);
		this->write(data32.byte2);
		this->write(data32.byte1);
		this->write(data32.lsb);
	}
}

__attribute__((weak)) void SPIClass::transferBytes(const uint8_t *data, uint8_t *out, uint32_t len) {
	while (len--) {
		*(out++) = this->transfer(*(data++));
	}
}

__attribute__((weak)) void SPIClass::writeBytes(const uint8_t *data, uint32_t len) {
	while (len--) {
		this->write(*(data++));
	}
}

// __attribute__((weak)) void SPIClass::transferBits(uint32_t data, uint32_t *out, uint8_t bits) {}
// __attribute__((weak)) void SPIClass::writePixels(const void *data, uint32_t len) {}
// __attribute__((weak)) void SPIClass::writePattern(const uint8_t *data, uint8_t len, uint32_t repeat) {}

#if LT_HW_SPI0
SPIClass SPI(0);
#elif LT_HW_SPI1
SPIClass SPI(1);
#elif LT_HW_SPI2
SPIClass SPI(2);
#endif

#if LT_HW_SPI0
SPIClass SPI0(PORT_FIXED_BIT | 0);
#endif
#if LT_HW_SPI1
SPIClass SPI1(PORT_FIXED_BIT | 1);
#endif
#if LT_HW_SPI2
SPIClass SPI2(PORT_FIXED_BIT | 2);
#endif

#endif
