/* Copyright (c) Kuba Szczodrzyński 2023-09-21. */

#pragma once

#if !LT_ARD_HAS_WIRE
#error "Wire library not implemented for this chip family"
#endif

#include <Arduino.h>

typedef struct WireData WireData;

// WIRE_HAS_BUFFER_SIZE means Wire has setBufferSize()
#undef WIRE_HAS_BUFFER_SIZE
// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END	  1
#define WIRE_DEFAULT_FREQ 100000

enum TwoWireResult {
	TWOWIRE_SUCCESS		= 0,
	TWOWIRE_TX_OVERFLOW = 1,
	TWOWIRE_NACK_ADDR	= 2,
	TWOWIRE_NACK_DATA	= 3,
	TWOWIRE_ERROR		= 4,
	TWOWIRE_TIMEOUT		= 5,
};

class TwoWire : public Stream {
  private:
	int port{-1};				 //!< port number, family-specific
	bool fixedPort{false};		 //!< whether port is not changeable
	pin_size_t sda{PIN_INVALID}; //!< SDA pin number of this instance
	pin_size_t scl{PIN_INVALID}; //!< SCL pin number of this instance

  private:
	WireData *data{nullptr};	//!< family-specific, created in begin(), destroyed in end()
	RingBuffer *rxBuf{nullptr}; //!< RX data buffer, assigned in begin(), removed in end()
	RingBuffer *txBuf{nullptr}; //!< TX data buffer, created in beginTransmission(), destroyed in endTransmission()
	uint32_t frequency{0};		//!< currently set clock frequency
	uint32_t timeout{50};		//!< I2C ACK timeout
	uint16_t address{0};		//!< slave address of this instance, or 0 if master
	uint16_t txAddress{0};		//!< target device address of current transmission

	void (*onRequestCallback)(void){nullptr};
	void (*onReceiveCallback)(int){nullptr};

	int getPortByPins(pin_size_t sda, pin_size_t scl);
	bool setPinsPrivate(pin_size_t sda, pin_size_t scl);

  private: /* family core */
	bool beginPrivate(uint8_t address, uint32_t frequency = 0);
	bool endPrivate();

  public: /* family core */
	bool setClock(uint32_t frequency);
	/* (master write) */
	TwoWireResult endTransmission(bool sendStop = true);
	/* (master read) */
	size_t requestFrom(uint16_t address, size_t len, bool sendStop);

  public: /* common core */
	TwoWire(uint32_t port) : port(port) {
		this->fixedPort = bool(port & PORT_FIXED_BIT);
	}

	TwoWire(pin_size_t sda, pin_size_t scl) : sda(sda), scl(scl) {}

	TwoWire() {}

	~TwoWire();

	bool begin(pin_size_t sda, pin_size_t scl, uint32_t frequency = 0);
	bool begin(uint8_t address, pin_size_t sda, pin_size_t scl, uint32_t frequency = 0);
	bool end();

	bool setPins(pin_size_t sda, pin_size_t scl);
	void beginTransmission(uint16_t address);
	size_t write(uint8_t data);
	size_t write(const uint8_t *data, size_t len);
	void flush();
	int available();
	int read();
	int peek();

	inline uint16_t getTimeOut() {
		return this->timeout;
	}

	inline void setTimeOut(uint16_t millis) {
		this->timeout = millis;
	}

	inline uint32_t getClock() {
		return this->frequency;
	}

	inline void onReceive(void (*callback)(int)) {
		this->onReceiveCallback = callback;
	}

	inline void onRequest(void (*callback)(void)) {
		this->onRequestCallback = callback;
	}

  public:
	inline bool begin() {
		return this->begin(PIN_INVALID, PIN_INVALID, static_cast<uint32_t>(0));
	}

	inline bool begin(uint8_t address) {
		return this->begin(address, PIN_INVALID, PIN_INVALID, 0);
	}

	inline bool begin(int address) {
		return this->begin(address, PIN_INVALID, PIN_INVALID, 0);
	}

  public:
	inline void beginTransmission(uint8_t address) {
		return this->beginTransmission(static_cast<uint16_t>(address));
	}

	inline void beginTransmission(int address) {
		return this->beginTransmission(static_cast<uint16_t>(address));
	}

  public:
	inline uint8_t requestFrom(uint16_t address, uint8_t len, bool sendStop) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), static_cast<bool>(sendStop));
	}

	inline uint8_t requestFrom(uint16_t address, uint8_t len, uint8_t sendStop) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), static_cast<bool>(sendStop));
	}

	inline uint8_t requestFrom(uint16_t address, uint8_t len) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), true);
	}

  public:
	inline size_t requestFrom(uint8_t address, size_t len, bool sendStop) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), static_cast<bool>(sendStop));
	}

	inline uint8_t requestFrom(uint8_t address, uint8_t len, uint8_t sendStop) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), static_cast<bool>(sendStop));
	}

	inline uint8_t requestFrom(uint8_t address, uint8_t len) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), true);
	}

	inline uint8_t requestFrom(int address, int len, int sendStop) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), static_cast<bool>(sendStop));
	}

	inline uint8_t requestFrom(int address, int len) {
		return this->requestFrom(static_cast<uint16_t>(address), static_cast<size_t>(len), true);
	}

  public:
	inline size_t write(const char *s) {
		return this->write((uint8_t *)s, strlen(s));
	}

	inline size_t write(unsigned long n) {
		return this->write((uint8_t)n);
	}

	inline size_t write(long n) {
		return this->write((uint8_t)n);
	}

	inline size_t write(unsigned int n) {
		return this->write((uint8_t)n);
	}

	inline size_t write(int n) {
		return this->write((uint8_t)n);
	}
};

#if LT_HW_I2C0 || LT_HW_I2C1 || LT_HW_I2C2
extern TwoWire Wire;
#endif

#if LT_HW_I2C0
extern TwoWire Wire0;
#endif
#if LT_HW_I2C1
extern TwoWire Wire1;
#endif
#if LT_HW_I2C2
extern TwoWire Wire2;
#endif
