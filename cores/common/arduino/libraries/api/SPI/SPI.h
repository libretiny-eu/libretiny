/* Copyright (c) Kuba Szczodrzyński 2023-09-23. */

#pragma once

#if !LT_ARD_HAS_SPI
#error "SPI library not implemented for this chip family"
#endif

#include <Arduino.h>

typedef struct SPIData SPIData;

#define SPI_MODE0 0 //!< CPOL=0, CPHA=0
#define SPI_MODE1 1 //!< CPOL=0, CPHA=1
#define SPI_MODE2 2 //!< CPOL=1, CPHA=0
#define SPI_MODE3 3 //!< CPOL=1, CPHA=1

#define SPI_LSBFIRST 0
#define SPI_MSBFIRST 1

#define SPI_HAS_TRANSACTION

class SPISettings {
  public:
	SPISettings() {}

	SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
		: _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}

	uint32_t _clock{1000000};
	uint8_t _bitOrder{SPI_MSBFIRST};
	uint8_t _dataMode{SPI_MODE0};
};

class SPIClass {
  private:
	uint8_t port{0};			  //!< port number, family-specific
	bool fixedPort{false};		  //!< whether port is not changeable
	pin_size_t sck{PIN_INVALID};  //!< SCK pin number of this instance
	pin_size_t miso{PIN_INVALID}; //!< MISO pin number of this instance
	pin_size_t mosi{PIN_INVALID}; //!< MOSI pin number of this instance
	pin_size_t cs{PIN_INVALID};	  //!< CS pin number of this instance

  private:
	SPIData *data{nullptr};	   //!< family-specific, created in begin(), destroyed in end()
	SPISettings settings{};	   //!< current SPI settings
	bool inTransaction{false}; //!< whether transaction is in progress
	bool useCs{false};		   //!< whether to toggle CS automatically

	uint8_t *txBuf{nullptr}; //!< TX data buffer (read head)
	uint32_t txLen{0};		 //!< TX data length (remaining)
	uint8_t *rxBuf{nullptr}; //!< RX data buffer (write head)
	uint32_t rxLen{0};		 //!< RX data length (remaining)

	int getPortByPins(pin_size_t sck, pin_size_t miso, pin_size_t mosi);
	bool setPinsPrivate(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs = PIN_INVALID);

  private: /* family core */
	bool beginPrivate();
	bool endPrivate();

  public: /* family core */
	void setFrequency(uint32_t frequency);
	void setDataMode(uint8_t dataMode);

	void commitTransaction();
	void isrHandler(void *param);
	static void isrHandlerStatic(void *param);

  public: /* common core */
	SPIClass(uint32_t port) : port(port) {
		this->fixedPort = bool(port & PORT_FIXED_BIT);
	}

	SPIClass(pin_size_t sck, pin_size_t miso, pin_size_t mosi) : sck(sck), miso(miso), mosi(mosi) {}

	~SPIClass();

	bool begin(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs = PIN_INVALID);
	bool begin();
	bool end();

	bool setPins(pin_size_t sck, pin_size_t miso, pin_size_t mosi, pin_size_t cs = PIN_INVALID);

	void beginTransaction(SPISettings settings);
	void endTransaction();
	void setHwCs(bool useCs);

	void setClockDivider(uint32_t clockDiv);
	uint32_t getClockDivider();
	void setBitOrder(uint8_t bitOrder);

	inline void transfer(void *data, uint32_t len) {
		this->transferBytes((const uint8_t *)data, (uint8_t *)data, len);
	}

	uint8_t transfer(uint8_t data);
	uint16_t transfer16(uint16_t data);
	uint32_t transfer32(uint32_t data);
	void write(uint8_t data);
	void write16(uint16_t data);
	void write32(uint32_t data);

	void transferBytes(const uint8_t *data, uint8_t *out, uint32_t len);
	void writeBytes(const uint8_t *data, uint32_t len);

	void transferBits(uint32_t data, uint32_t *out, uint8_t bits);
	void writePixels(const void *data, uint32_t len);
	void writePattern(const uint8_t *data, uint8_t len, uint32_t repeat);

	int8_t pinSS() {
		return (int8_t)this->cs;
	}
};

#if LT_HW_SPI0 || LT_HW_SPI1 || LT_HW_SPI2
extern SPIClass SPI;
#endif

#if LT_HW_SPI0
extern SPIClass SPI0;
#endif
#if LT_HW_SPI1
extern SPIClass SPI1;
#endif
#if LT_HW_SPI2
extern SPIClass SPI2;
#endif
