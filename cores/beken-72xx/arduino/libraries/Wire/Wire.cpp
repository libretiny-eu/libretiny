/* Copyright (c) Kuba Szczodrzyński 2023-12-08. */

#include "WirePrivate.h"

#define I2C_DIVID_CLK(div) (I2C1_DEFAULT_CLK / (((div + 1) * 3) - 6))
#define I2C_BUSY_RETRIES   5

enum I2CISRMode {
	I2C_ISR_IDLE		 = 0,
	I2C_ISR_MASTER_WRITE = 1,
	I2C_ISR_MASTER_READ	 = 2,
	I2C_ISR_SLAVE_WRITE	 = 3,
	I2C_ISR_SLAVE_READ	 = 4,
};

static I2CISRMode i2c1Mode	 = I2C_ISR_IDLE;
static bool i2c1SendStop	 = false;
static bool i2c1GotAck		 = false;
static RingBuffer *i2c1TxBuf = nullptr;
static RingBuffer *i2c1RxBuf = nullptr;
static int32_t i2c1RxLength	 = 0;

static I2CISRMode i2c2Mode	 = I2C_ISR_IDLE;
static bool i2c2SendStop	 = false;
static bool i2c2GotAck		 = false;
static RingBuffer *i2c2TxBuf = nullptr;
static RingBuffer *i2c2RxBuf = nullptr;
static int32_t i2c2RxLength	 = 0;

static void i2c1Handler();
static void i2c2Handler();

#define REG_I2C2_READ_ALL(x)                                                                                           \
	x.config = REG_I2C2->config;                                                                                       \
	x.status = REG_I2C2->status;

#define REG_I2C2_WRITE_ALL(x)                                                                                          \
	REG_I2C2->status = x.status;                                                                                       \
	REG_I2C2->config = x.config;

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
			REG_I2C1->config = 0;
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
			REG_I2C2->config = 0;
			REG_I2C2->status = 0;
			intc_service_register(IRQ_I2C2, PRI_IRQ_I2C2, i2c2Handler);
			// i2c2_set_idle_cr(0x3)
			REG_I2C2->IDLE_CR = 3;
			// i2c2_set_scl_cr(0x4)
			REG_I2C2->SCL_CR = 4;
			// i2c2_set_smbus_cs(0x3)
			REG_I2C2->SMBCS = 0b11;
			// i2c2_set_timeout_en(1)
			REG_I2C2->SMBTOE = true;
			// i2c2_set_free_detect(1)
			REG_I2C2->SMBFTE = true;
			// i2c2_set_salve_en(0)
			REG_I2C2->INH = true;
			// i2c2_power_up()
			param = PCLK_POSI_I2C2;
			icu_ctrl(CMD_CONF_PCLK_26M, &param);
			param = PWD_I2C2_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_UP, &param);
			// i2c1_enable_interrupt()
			param = IRQ_I2C2_BIT;
			icu_ctrl(CMD_ICU_INT_ENABLE, &param);
			// i2c2_gpio_config()
			param = GFUNC_MODE_I2C2;
			gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);
			break;
	}

	// TODO support I2C2 and slave mode
	this->address = address;

	return true;
}

/* if (REG_I2C1->BUSY) {
	if (this->data->busyCount >= I2C_BUSY_RETRIES) {
		LT_FM(I2C, "GPIO didn't help!");
		return false;
	} else {
		LT_EM(
			I2C,
			"Busy in begin()! GPIO taking over (%u/" STRINGIFY_MACRO(I2C_BUSY_RETRIES) ")",
			++this->data->busyCount
		);
		this->endPrivate();
		delay(200);
		pinMode(this->sda, OUTPUT);
		pinMode(this->scl, OUTPUT);
		digitalWrite(this->sda, HIGH);
		digitalWrite(this->scl, HIGH);
		delay(300);
		return this->beginPrivate(address, frequency);
	}
} */

bool TwoWire::setClock(uint32_t frequency) {
	if (!this->data)
		return false;

	uint32_t maxFrequency = 400000;
	if (frequency > maxFrequency) {
		LT_WM(I2C, "Clock freq too high! %lu < %lu", frequency, maxFrequency);
		frequency = maxFrequency;
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
			REG_I2C1->FREQ_DIV = div;
			break;
		case 2:
			REG_I2C2->FREQ_DIV = div;
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
			REG_I2C1->ENSMB = false;
			// i2c1_disable_interrupt()
			param = IRQ_I2C1_BIT;
			icu_ctrl(CMD_ICU_INT_DISABLE, &param);
			// i2c1_power_down()
			param = PWD_I2C1_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_DOWN, &param);
			// i2c1_exit()
			REG_I2C1->config = 0;
			break;
		case 2:
			// i2c2_set_ensmb(0)
			REG_I2C2->ENSMB = false;
			// i2c2_disable_interrupt()
			param = IRQ_I2C2_BIT;
			icu_ctrl(CMD_ICU_INT_DISABLE, &param);
			// i2c2_power_down()
			param = PWD_I2C2_CLK_BIT;
			icu_ctrl(CMD_CLK_PWR_DOWN, &param);
			// i2c2_exit()
			REG_I2C2->config = 0;
			REG_I2C2->status = 0;
			break;
	}

	return true;
}

/* LT_DM(
	I2C,
	"ISR IN : mode=%d, tx=%d/rx=%d, ENSMB=%d, STA=%d, STO=%d, ACKTX=%d, TXMODE=%d, ACKRX=%d, ACKRQ=%d",
	i2c1Mode,
	i2c1TxBuf ? i2c1TxBuf->available() : -1,
	i2c1RxLength,
	REG_I2C1->ENSMB,
	REG_I2C1->STA,
	REG_I2C1->STO,
	REG_I2C1->ACKTX,
	REG_I2C1->TXMODE,
	REG_I2C1->ACKRX,
	REG_I2C1->ACKRQ
); */

static void i2c1Handler() {
	if (!REG_I2C1->SI) {
		LT_WM(I2C, "I2C1 interrupt not triggered");
		return;
	}

	if (REG_I2C1->TXMODE) {
		// check if ACK received
		i2c1GotAck = REG_I2C1->ACKRX;
		// end the transmission if NACK
		if (!i2c1GotAck)
			goto stop;
	}

	switch (i2c1Mode) {
		case I2C_ISR_MASTER_WRITE:
			// end the transmission if no data left to send
			if (i2c1TxBuf == nullptr || i2c1TxBuf->available() == 0)
				goto stop;
			// otherwise write the next byte
			REG_I2C1->SMB_DAT = i2c1TxBuf->read_char();
			break;

		case I2C_ISR_MASTER_READ:
			// disable TX mode unconditionally
			REG_I2C1->TXMODE = false;
			// failsafe for no buffer
			if (i2c1RxBuf == nullptr)
				i2c1RxLength = 0;
			// quit the first ISR call, wait for next one with data
			if (REG_I2C1->STA) {
				// send ACK if *any* data is requested
				REG_I2C1->ACKTX = i2c1RxLength > 0 ? true : false;
				goto end;
			} else {
				// send ACK if *more* data is requested
				REG_I2C1->ACKTX = i2c1RxLength > 1 ? true : false;
			}
			// receive data
			if (i2c1RxLength > 0) {
				i2c1RxBuf->store_char(REG_I2C1->SMB_DAT);
				i2c1RxLength--;
			}
			// end the transmission if no data left to receive
			if (i2c1RxLength <= 0)
				goto stop;
			break;
	}
	goto end;

stop:
	// both methods will stop firing the ISR:
	if (i2c1SendStop) {
		// - send STOP condition if requested
		// REG_I2C1->TXMODE = true;
		REG_I2C1->STO = true;
	} else {
		// - otherwise disable I2C entirely
		REG_I2C1->ENSMB = false;
	}
	// finish the transmission
	i2c1Mode = I2C_ISR_IDLE;

end:
	REG_I2C1->STA = false;
	REG_I2C1->SI  = false;
}

static void i2c2Handler() {
	if (!REG_I2C2->SI) {
		LT_WM(I2C, "I2C2 interrupt not triggered");
		return;
	}

	i2c2_hw_t REG_I2C;
	REG_I2C2_READ_ALL(REG_I2C);

	/* LT_DM(
		I2C,
		"ISR IN : mode=%d, tx=%d/rx=%d, ENSMB=%d, STA=%d, STO=%d, ACK=%d, TXMODE=%d, ACKRQ=%d",
		i2c2Mode,
		i2c2TxBuf ? i2c2TxBuf->available() : -1,
		i2c2RxLength,
		REG_I2C.ENSMB,
		REG_I2C.STA,
		REG_I2C.STO,
		REG_I2C.ACK,
		REG_I2C.TXMODE,
		REG_I2C.ACKRQ
	); */

	if (REG_I2C.TXMODE) {
		// check if ACK received
		i2c2GotAck = REG_I2C.ACK;
		// end the transmission if NACK
		if (!i2c2GotAck)
			goto stop;
	}

	switch (i2c2Mode) {
		case I2C_ISR_MASTER_WRITE:
			// end the transmission if no data left to send
			if (i2c2TxBuf == nullptr || i2c2TxBuf->available() == 0)
				goto stop;
			// otherwise write the next byte
			REG_I2C2->SMB_DAT = i2c2TxBuf->read_char();
			break;

		case I2C_ISR_MASTER_READ:
			// failsafe for no buffer
			if (i2c2RxBuf == nullptr)
				i2c2RxLength = 0;
			// quit the first ISR call, wait for next one with data
			if (REG_I2C.STA) {
				// send ACK if *any* data is requested
				REG_I2C.ACK = i2c2RxLength > 0 ? true : false;
				goto end;
			} else {
				// send ACK if *more* data is requested
				REG_I2C.ACK = i2c2RxLength > 1 ? true : false;
			}
			// receive data
			if (i2c2RxLength > 0) {
				i2c2RxBuf->store_char(REG_I2C2->SMB_DAT);
				i2c2RxLength--;
			}
			// end the transmission if no data left to receive
			if (i2c2RxLength <= 0)
				goto stop;
			break;
	}
	goto end;

stop:
	// both methods will stop firing the ISR:
	if (i2c2SendStop) {
		// - send STOP condition if requested
		REG_I2C.STO = true;
	} else {
		// - otherwise disable I2C entirely
		// REG_I2C2->ENSMB = false;
	}
	// finish the transmission
	i2c2Mode = I2C_ISR_IDLE;

end:
	REG_I2C.STA = false;
	REG_I2C.SI	= false;
	REG_I2C2_WRITE_ALL(REG_I2C);

	/* LT_DM(
		I2C,
		"ISR OUT: mode=%d, tx=%d/rx=%d, ENSMB=%d, STA=%d, STO=%d, ACK=%d, TXMODE=%d, ACKRQ=%d",
		i2c2Mode,
		i2c2TxBuf ? i2c2TxBuf->available() : -1,
		i2c2RxLength,
		REG_I2C.ENSMB,
		REG_I2C.STA,
		REG_I2C.STO,
		REG_I2C.ACK,
		REG_I2C.TXMODE,
		REG_I2C.ACKRQ
	); */
}

TwoWireResult TwoWire::endTransmission(bool sendStop) {
	if (!this->txBuf || this->txAddress == 0x00)
		return TWOWIRE_ERROR;
	RingBuffer *buf = this->txBuf;

	i2c2_hw_t REG_I2C;
	int dataLength;
	I2CISRMode *i2cModePtr;
	bool *i2cGotAckPtr;

	portDISABLE_INTERRUPTS();
	switch (this->port) {
		case 1:
			// disable STOP condition
			REG_I2C1->STO = false;
			// set slave address
			REG_I2C1->SMB_DAT = (this->txAddress << 1) | 0;
			// enable START condition
			REG_I2C1->STA = true;
			// set work state for ISR
			dataLength	 = buf->available();
			i2c1Mode	 = I2C_ISR_MASTER_WRITE;
			i2c1SendStop = sendStop;
			i2c1TxBuf	 = buf;
			i2cModePtr	 = &i2c1Mode;
			i2cGotAckPtr = &i2c1GotAck;
			// start the transmission and I2C peripheral
			REG_I2C1->TXMODE = true;
			REG_I2C1->ENSMB	 = true;
			break;
		case 2:
			REG_I2C2_READ_ALL(REG_I2C);
			// disable STOP condition
			REG_I2C.STO = false;
			// set slave address
			REG_I2C2->SMB_DAT = (this->txAddress << 1) | 0;
			// enable START condition
			REG_I2C.STA = true;
			// set work state for ISR
			dataLength	 = buf->available();
			i2c2Mode	 = I2C_ISR_MASTER_WRITE;
			i2c2SendStop = sendStop;
			i2c2TxBuf	 = buf;
			i2cModePtr	 = &i2c2Mode;
			i2cGotAckPtr = &i2c2GotAck;
			// start the transmission and I2C peripheral
			REG_I2C.ENSMB = true;
			REG_I2C2_WRITE_ALL(REG_I2C);
			break;
		default:
			portENABLE_INTERRUPTS();
			return TWOWIRE_ERROR;
	}
	portENABLE_INTERRUPTS();

	// wait for transmission to finish
	unsigned long start = millis();
	while (*i2cModePtr != I2C_ISR_IDLE) {
		ps_delay(1000);
		if (millis() - start > timeout) {
			LT_EM(I2C, "Timeout @ 0x%02x (TX not done)", this->txAddress);
			return TWOWIRE_TIMEOUT;
		}
	}

	// stop the transmission (wait a bit for STOP condition)
	delayMicroseconds(500);
	switch (this->port) {
		case 1:
			REG_I2C1->TXMODE = false;
			REG_I2C1->ENSMB	 = false;
			break;
		case 2:
			REG_I2C2->ENSMB = false;
			break;
	}

	// check if ACK received
	if (!*i2cGotAckPtr) {
		if (buf->available() == dataLength)
			return TWOWIRE_NACK_ADDR;
		return TWOWIRE_NACK_DATA;
	}
	return TWOWIRE_SUCCESS;
}

size_t TwoWire::requestFrom(uint16_t address, size_t len, bool sendStop) {
	if (!this->rxBuf || this->address != 0x00)
		return 0;
	RingBuffer *buf = this->rxBuf;

	I2CISRMode *i2cModePtr;
	int32_t *i2cRxLengthPtr;

	portDISABLE_INTERRUPTS();
	switch (this->port) {
		case 1:
			// disable STOP condition
			REG_I2C1->STO = false;
			// set slave address
			REG_I2C1->SMB_DAT = (this->txAddress << 1) | 1;
			// enable START condition
			REG_I2C1->STA = true;
			// set work state for ISR
			i2c1Mode	   = I2C_ISR_MASTER_READ;
			i2c1SendStop   = sendStop;
			i2c1RxBuf	   = buf;
			i2c1RxLength   = len;
			i2cModePtr	   = &i2c1Mode;
			i2cRxLengthPtr = &i2c1RxLength;
			// start the transmission and I2C peripheral
			REG_I2C1->TXMODE = true;
			REG_I2C1->ENSMB	 = true;
			break;
		case 2:
			// disable STOP condition
			REG_I2C2->STO = false;
			// set slave address
			REG_I2C2->SMB_DAT = (this->txAddress << 1) | 1;
			// enable START condition
			REG_I2C2->STA = true;
			// set work state for ISR
			i2c2Mode	   = I2C_ISR_MASTER_READ;
			i2c2SendStop   = sendStop;
			i2c2RxBuf	   = buf;
			i2c2RxLength   = len;
			i2cModePtr	   = &i2c2Mode;
			i2cRxLengthPtr = &i2c2RxLength;
			// start the transmission and I2C peripheral
			REG_I2C2->TXMODE = true;
			REG_I2C2->ENSMB	 = true;
			break;
		default:
			portENABLE_INTERRUPTS();
			return 0;
	}
	portENABLE_INTERRUPTS();

	// wait for transmission to finish
	unsigned long start = millis();
	while (*i2cModePtr != I2C_ISR_IDLE) {
		ps_delay(1000);
		if (millis() - start > timeout) {
			LT_EM(I2C, "Timeout @ 0x%02x (RX not done)", this->txAddress);
			goto end;
		}
	}

	// stop the transmission (wait a bit for STOP condition)
	delayMicroseconds(500);
	switch (this->port) {
		case 1:
			REG_I2C1->TXMODE = false;
			REG_I2C1->ENSMB	 = false;
			break;
		case 2:
			REG_I2C2->TXMODE = false;
			REG_I2C2->ENSMB	 = false;
			break;
	}

end:
	return len - *i2cRxLengthPtr;
}
