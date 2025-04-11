/* Copyright (c) Etienne Le Cousin 2025-01-19. */

#include "wiring_private.h"
#include <sdk_private.h>

#define I2C_PRIV i2c_init_t_def
#include "Wire.h"

// Functions from I2C demo of SDK
static uint8_t hal_i2c_master_7bit_write(uint32_t i2c_x_base, uint8_t dev_addr, const uint8_t *buf, uint16_t buf_len);
static uint8_t hal_i2c_master_7bit_read(uint32_t i2c_x_base, uint8_t dev_addr, uint8_t *buf, uint16_t buf_len);

#ifdef PIN_WIRE0_SDA
// Wire object associated to I2C0 interface.
TwoWire Wire(PIN_WIRE0_SDA, PIN_WIRE0_SCL);
#endif

TwoWire::TwoWire(int8_t sda, int8_t scl) {
	_sda = sda;
	_scl = scl;
}

TwoWire::~TwoWire() {}

bool TwoWire::setPins(int8_t sda, int8_t scl) {
	// return true when changing pins on initialized I2C
	if (_inSetPins)
		return true;
	// check if pins are provided
	if (sda == -1 || scl == -1)
		return false;
	// set private pins
	_sda			 = sda;
	_scl			 = scl;
	uint32_t pin_sda = pinInfo(sda)->gpio;
	uint32_t pin_scl = pinInfo(scl)->gpio;

	hal_gpio_pin_afio_select(GPIO_GET_BASE(pin_sda), GPIO_GET_PIN(pin_sda), I2C0_SDA); // TODO: check pin value
	hal_gpio_pin_afio_select(GPIO_GET_BASE(pin_scl), GPIO_GET_PIN(pin_scl), I2C0_SCL);
	hal_gpio_pin_afio_en(GPIO_GET_BASE(pin_sda), GPIO_GET_PIN(pin_sda), HAL_ENABLE);
	hal_gpio_pin_afio_en(GPIO_GET_BASE(pin_scl), GPIO_GET_PIN(pin_scl), HAL_ENABLE);

	// restart I2C if changing pins
	// this will never be called from begin()
	if (_i2c) {
		_inSetPins = true;
		end();
		begin();
		_inSetPins = false;
	}
	return true;
}

bool TwoWire::begin(int8_t sda, int8_t scl, uint32_t frequency) {
	if (_i2c)
		return true;
	// set private i2c pins
	if (!setPins(sda, scl))
		return false;
	// use default frequency
	if (!frequency)
		frequency = WIRE_DEFAULT_FREQ;

	/* disable the i2c */
	hal_i2c_en(I2C_BASE, HAL_DISABLE);

	_freq = frequency;
	_i2c  = new i2c_init_t_def;
	memset(&_i2c, 0, sizeof(_i2c));
	_i2c->i2c_peripheral_clock_freq = 4;
	_i2c->i2c_master_mode_sel		= I2C_FM_MODE;
	_i2c->i2c_fm_mode_duty_cycle	= I2C_FM_MODE_DUTY_CYCLE_2;
	/*
	 * TPCLK1 = 1/80MHz = 0.0125us
	 * Thigh = 9  x CCR x TPCLK1
	 * Tlow  = 16 x CCR x TPCLK1
	 * Thigh + Tlow = 1/frequency
	 * ccr =  3.200.000/frequency
	 */
	_i2c->i2c_ccr	= 3200000 / frequency;
	_i2c->i2c_trise = 0xF;

	hal_i2c_init(I2C_BASE, _i2c);

	hal_i2c_en(I2C_BASE, HAL_ENABLE);
	return true;
}

bool TwoWire::begin(uint8_t address, int8_t sda, int8_t scl, uint32_t frequency) {
	if (_i2c)
		return true;
	// init master bus first, return if failed
	if (!begin(sda, scl, frequency))
		return false;

	hal_i2c_slave_set_add_mode(I2C_BASE, I2C_ADD_7BIT_MODE);
	hal_i2c_slave_set_add1(I2C_BASE, address);
	return true;
}

bool TwoWire::end() {
	hal_i2c_deinit();
	delete _i2c;
	_i2c = NULL;
	return true;
}

bool TwoWire::setClock(uint32_t freq) {
	if (_i2c) {
		_i2c->i2c_ccr = 3200000 / freq;
		hal_i2c_init(I2C_BASE, _i2c);
	}
	_freq = freq;
	return true;
}

void TwoWire::beginTransmission(uint8_t address) {
	_txAddr = address;
	_txBuf.clear();
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit) {
	if (!_i2c || !_txAddr)
		return 4;
	uint8_t *buf = (uint8_t *)malloc(_txBuf.available());
	uint8_t i	 = 0;
	while (_txBuf.available()) {
		buf[i++] = _txBuf.read_char();
	}
	if (hal_i2c_master_7bit_write(I2C_BASE, _txAddr, buf, i) == HAL_RESET)
		return 4;
	free(buf);
	_txAddr = 0;
	return 0;
}

size_t TwoWire::requestFrom(uint8_t address, size_t len, bool stopBit) {
	if (!len)
		return 0;

	if (len > SERIAL_BUFFER_SIZE)
		len = SERIAL_BUFFER_SIZE;

	_rxBuf.clear();

	uint8_t *buf = (uint8_t *)malloc(_txBuf.available());
	if (hal_i2c_master_7bit_read(I2C_BASE, address, buf, len) == HAL_RESET)
		return 0;

	uint8_t i = 0;
	while (len) {
		_rxBuf.store_char(buf[i++]);
		len--;
	}
	free(buf);
	return len;
}

size_t TwoWire::write(uint8_t data) {
	if (!_txAddr || _txBuf.isFull())
		return 0;
	_txBuf.store_char(data);
	return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (!write(data[i]))
			return i;
	}
	return len;
}

int TwoWire::available() {
	return _rxBuf.available();
}

int TwoWire::read() {
	return _rxBuf.read_char();
}

int TwoWire::peek() {
	return _rxBuf.peek();
}

void TwoWire::flush() {}

//--------------------------------------------------------------------------------------------------------------------
#define TIMEOUT_CYCLE 4000

static uint8_t hal_i2c_master_7bit_write(uint32_t i2c_x_base, uint8_t dev_addr, const uint8_t *buf, uint16_t buf_len) {
	// 1. check busy
	if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET) {
		hal_i2c_master_reset(i2c_x_base);
		return HAL_RESET;
	}

	// 2. send start
	if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
		return HAL_RESET;

	// 3. send addr
	hal_i2c_master_send_data(i2c_x_base, dev_addr);

	// 4. wait send complete
	if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
		return HAL_RESET;

	// 5. clear addr flag
	hal_i2c_clear_sr(i2c_x_base);

	// 6. send data
	for (uint32_t i = 0; i < buf_len; i++) {
		// wait tx empty flag
		if (hal_i2c_wait_txe(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET) {
			return HAL_RESET;
		} else {
			hal_i2c_master_send_data(i2c_x_base, buf[i]);
		}
	}

	// 7. wait send complete.
	if (hal_i2c_wait_btf(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
		return HAL_RESET;

	// 8. stop the i2c.
	hal_i2c_master_stop(i2c_x_base);

	return HAL_SET;
}

static uint8_t hal_i2c_master_7bit_read(uint32_t i2c_x_base, uint8_t dev_addr, uint8_t *buf, uint16_t buf_len) {
	// 1. check busy
	if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET) {
		hal_i2c_master_reset(i2c_x_base);
		return HAL_RESET;
	}

	// 2. send start
	if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
		return HAL_RESET;

	// 3. send addr (+1 is read operation)
	hal_i2c_master_send_data(i2c_x_base, dev_addr + 1);

	// 4. Wait for an ack after sending the address
	if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
		return HAL_RESET;

	// 5. clear addr flag
	hal_i2c_clear_sr(i2c_x_base);

	// 6. clear the DR
	hal_i2c_master_recv_data(i2c_x_base);

	// 7. receive data
	for (int i = buf_len; i > 0; i--) {
		// when reading the last byte,do not send the ack
		if (buf_len == 1) {
			// do not send the ack
			hal_i2c_ack_en(i2c_x_base, HAL_DISABLE);

			/// wait rx not empty
			if (hal_i2c_wait_rxne(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET) {
				return HAL_RESET;
			} else {
				*buf = hal_i2c_master_recv_data(i2c_x_base);
			}
			// read data
		} else {
			// send ack
			hal_i2c_ack_en(i2c_x_base, HAL_ENABLE);

			// wait rx not empty
			if (hal_i2c_wait_rxne(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET) {
				return HAL_RESET;
			} else {
				*buf = hal_i2c_master_recv_data(i2c_x_base);
			}
		}
		buf_len--;
		buf++;
	}

	// 8. stop the i2c.
	hal_i2c_master_stop(i2c_x_base);

	return HAL_SET;
}
