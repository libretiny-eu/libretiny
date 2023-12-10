/* Copyright (c) Kuba Szczodrzyński 2023-12-08. */

#include "WirePrivate.h"

#define I2C_DIVID_CLK(div) (I2C1_DEFAULT_CLK / (((div + 1) * 3) - 6))

enum I2CISRMode {
	I2C_ISR_NONE		 = 0,
	I2C_ISR_MASTER_WRITE = 1,
	I2C_ISR_MASTER_READ	 = 2,
	I2C_ISR_SLAVE_WRITE	 = 3,
	I2C_ISR_SLAVE_READ	 = 4,
};

static I2CISRMode i2c1Mode	 = I2C_ISR_NONE;
static RingBuffer *i2c1TxBuf = nullptr;
static bool i2c1TxSendStop	 = false;
static bool i2c1TxDone		 = false;
static bool i2c1TxAck		 = false;

static void i2c1Handler();

bool TwoWire::beginPrivate(uint8_t address, uint32_t frequency) {
	if (!this->data)
		return false;
	this->data->buf = this->rxBuf;

	if (this->port == 1 && address != 0x00)
		// I2C1 supports master mode only
		return false;

	LT_BUILD_CHECK(sizeof(*REG_I2C1) == 8);
	LT_BUILD_CHECK(sizeof(*REG_I2C2) == 12);

	uint32_t reg, param;
	switch (this->port) {
		case 1:
			// i2c1_hardware_init()
			REG_I2C1->config.value = 0;
			intc_service_register(IRQ_I2C1, PRI_IRQ_I2C1, i2c1Handler);
			// i2c1_power_up()
			param = PWD_I2C1_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_UP, &param);
			param = PCLK_POSI_I2C1;
			icu_ctrl(CMD_CONF_PCLK_26M, &param);
			// i2c1_enable_interrupt()
			param = IRQ_I2C1_BIT;
			icu_ctrl(CMD_ICU_INT_ENABLE, &param);
			// i2c1_gpio_config()
			param = GFUNC_MODE_I2C1;
			gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);
			break;
		case 2:
			// i2c2_hardware_init()
			REG_I2C2->config.value = 0;
			REG_I2C2->status.value = 0;
			// intc_service_register(IRQ_I2C2, PRI_IRQ_I2C2, i2c2Handler);
			// i2c2_set_idle_cr(0x3)
			REG_I2C2->config.idle_cr = 3;
			// i2c2_set_scl_cr(0x4)
			REG_I2C2->config.scl_cr = 4;
			// i2c2_set_smbus_cs(0x3)
			REG_I2C2->config.clk_src = 3;
			// i2c2_set_timeout_en(1)
			REG_I2C2->config.timeout_en = 1;
			// i2c2_set_free_detect(1)
			REG_I2C2->config.idle_det_en = 1;
			// i2c2_set_salve_en(0)
			REG_I2C2->config.inh = 0;
			// i2c2_power_up()
			param = PCLK_POSI_I2C2;
			icu_ctrl(CMD_CONF_PCLK_26M, &param);
			param = PWD_I2C2_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_UP, &param);
			// i2c2_gpio_config()
			param = GFUNC_MODE_I2C2;
			gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);
			break;
	}

	// TODO support I2C2 and slave mode
	this->address = address;

	return true;
}

bool TwoWire::setClock(uint32_t frequency) {
	if (!this->data)
		return false;

	if (frequency >= 50000 && frequency < 400000) {
		// this frequency range is very unreliable on BK7231
		frequency = 400000;
	}

	uint32_t div = I2C_CLK_DIVID(frequency);
	if (div > I2C1_FREQ_DIV_MASK) {
		uint32_t minFrequency = I2C_DIVID_CLK(1023);
		LT_WM(I2C, "Clock freq too low! %lu < %lu", frequency, minFrequency);
		div = I2C1_FREQ_DIV_MASK;
	}

	uint32_t reg;
	switch (this->port) {
		case 1:
			REG_I2C1->config.freq_div = div;
			break;
		case 2:
			REG_I2C2->config.freq_div = div;
			break;
	}

	this->frequency = frequency;
	return true;
}

bool TwoWire::endPrivate() {
	if (!this->data)
		return true;

	uint32_t reg, param;
	switch (this->port) {
		case 1:
			// i2c1_set_ensmb(0)
			REG_I2C1->config.en = false;
			// i2c1_disable_interrupt()
			param = IRQ_I2C1_BIT;
			icu_ctrl(CMD_ICU_INT_DISABLE, &param);
			// i2c1_power_down()
			param = PWD_I2C1_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_DOWN, &param);
			// i2c1_exit()
			REG_I2C1->config.value = 0;
			break;
		case 2:
			// i2c2_set_ensmb(0)
			REG_I2C2->config.en = false;
			// i2c2_disable_interrupt()
			param = IRQ_I2C2_BIT;
			icu_ctrl(CMD_ICU_INT_DISABLE, &param);
			// i2c2_power_down()
			param = PWD_I2C2_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_DOWN, &param);
			// i2c2_exit()
			REG_I2C2->config.value = 0;
			REG_I2C2->status.value = 0;
			break;
	}

	return true;
}

static void i2c1Handler() {
	if (!REG_I2C1->config.sm_int) {
		LT_WM(I2C, "I2C1 interrupt not triggered");
		return;
	}

	// LT_D(
	// 	"In ISR: mode=%d, available=%d, config=%08x/%08x",
	// 	i2c1Mode,
	// 	i2c1TxBuf ? i2c1TxBuf->available() : -1,
	// 	LT_MEM32(REG_I2C1_CONFIG),
	// 	REG_I2C1->config.value
	// );

	// uint32_t reg;
	switch (i2c1Mode) {
		case I2C_ISR_MASTER_WRITE:
			// disable START condition
			if (REG_I2C1->config.start) {
				REG_I2C1->config.start = false;
			}
			// check if ACK received
			i2c1TxAck = REG_I2C1->config.rx_ack;
			// end the transmission if NACK or no data left
			if (!i2c1TxAck || i2c1TxBuf == nullptr || i2c1TxBuf->available() == 0) {
				// send STOP condition if requested
				if (i2c1TxSendStop)
					REG_I2C1->config.stop = true;
				// finish the transmission
				i2c1Mode   = I2C_ISR_NONE;
				i2c1TxDone = true;
				break;
			}
			// otherwise write the next byte
			REG_I2C1->data.data = i2c1TxBuf->read_char();
			break;
	}

	REG_I2C1->config.sm_int = false;
}

TwoWireResult TwoWire::endTransmission(bool sendStop) {
	if (!this->txBuf || this->txAddress == 0x00)
		return TWOWIRE_ERROR;
	RingBuffer *buf = this->txBuf;
	uint32_t reg;

	portDISABLE_INTERRUPTS();
	// disable STOP condition
	REG_I2C1->config.stop = false;
	// set slave address
	REG_I2C1->data.data = (this->txAddress << 1) | 0;
	// enable START condition
	REG_I2C1->config.start = true;
	// set work state for ISR
	int dataLength = buf->available();
	i2c1Mode	   = I2C_ISR_MASTER_WRITE;
	i2c1TxBuf	   = buf;
	i2c1TxSendStop = sendStop;
	i2c1TxDone	   = false;
	i2c1TxAck	   = false;
	// start the transmission and I2C peripheral
	REG_I2C1->config.tx_mode = true;
	REG_I2C1->config.en		 = true;
	portENABLE_INTERRUPTS();

	// wait for transmission to finish
	unsigned long start = millis();
	while (!i2c1TxDone) {
		ps_delay(1000);
		if (millis() - start > timeout) {
			LT_EM(I2C, "Timeout @ 0x%02x (TX not done)", this->txAddress);
			return TWOWIRE_TIMEOUT;
		}
	}

	// stop the transmission (wait a bit for STOP condition)
	ps_delay(1000);
	REG_I2C1->config.tx_mode = false;
	REG_I2C1->config.en		 = false;

	// check if ACK received
	if (!i2c1TxAck) {
		if (buf->available() == dataLength)
			return TWOWIRE_NACK_ADDR;
		return TWOWIRE_NACK_DATA;
	}
	return TWOWIRE_SUCCESS;
}
