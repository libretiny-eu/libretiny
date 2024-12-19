/* Copyright (c) Kuba Szczodrzyński 2023-09-21. */

#include "WirePrivate.h"

bool TwoWire::beginPrivate(uint8_t address, uint32_t frequency) {
	if (!this->data)
		return false;
	this->data->buf = this->rxBuf;
	this->data->i2c = I2C_DEV_TABLE[this->port].I2Cx;

	switch (this->port) {
		case 0:
			RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);
			break;
		case 1:
			RCC_PeriphClockCmd(APBPeriph_I2C1, APBPeriph_I2C1_CLOCK, ENABLE);
			break;
	}

	Pinmux_Config(this->sda, PINMUX_FUNCTION_I2C);
	Pinmux_Config(this->scl, PINMUX_FUNCTION_I2C);
	PAD_PullCtrl(this->sda, GPIO_PuPd_UP);
	PAD_PullCtrl(this->scl, GPIO_PuPd_UP);

	I2C_InitTypeDef *init = &this->data->init;
	I2C_StructInit(init);
	init->I2CIdx	 = this->port;
	init->I2CMaster	 = address == 0x00 ? I2C_MASTER_MODE : I2C_SLAVE_MODE;
	init->I2CAckAddr = address;
	this->address	 = address;

	return true;
}

bool TwoWire::setClock(uint32_t frequency) {
	if (!this->data)
		return false;
	I2C_InitTypeDef *init = &this->data->init;
	I2C_TypeDef *i2c	  = this->data->i2c;
	uint32_t freqKhz	  = frequency / 1000;

	I2C_Cmd(i2c, DISABLE);

	if (freqKhz <= 100)
		init->I2CSpdMod = I2C_SS_MODE;
	else if (freqKhz <= 400)
		init->I2CSpdMod = I2C_FS_MODE;
	else
		init->I2CSpdMod = I2C_HS_MODE;
	init->I2CClk = freqKhz;

	I2C_Init(i2c, init);
	I2C_Cmd(i2c, ENABLE);

	this->frequency = frequency;
	return true;
}

bool TwoWire::endPrivate() {
	if (!this->data)
		return true;
	I2C_TypeDef *i2c = this->data->i2c;

	I2C_Cmd(i2c, DISABLE);
	return true;
}

static bool isFlagTimeout(I2C_TypeDef *i2c, uint32_t timeout, uint32_t flag) {
	unsigned long start = millis();
	while ((I2C_CheckFlagState(i2c, flag)) == 0) {
		DelayUs(2);
		if (millis() - start > timeout) {
			// reset I2C to clear trap state
			DelayUs(100);
			I2C_Cmd(i2c, DISABLE);
			I2C_Cmd(i2c, ENABLE);
			return true;
		}
	}
	return false;
}

static bool isNoAck(I2C_TypeDef *i2c) {
	if (I2C_GetRawINT(i2c) & BIT_IC_RAW_INTR_STAT_TX_ABRT) {
		// received NACK
		I2C_ClearAllINT(i2c);
		// reset I2C to clear trap state
		DelayUs(100);
		I2C_Cmd(i2c, DISABLE);
		I2C_Cmd(i2c, ENABLE);
		return true;
	}
	return false;
}

TwoWireResult TwoWire::endTransmission(bool sendStop) {
	if (!this->txBuf || this->txAddress == 0x00)
		return TWOWIRE_ERROR;
	I2C_InitTypeDef *init = &this->data->init;
	I2C_TypeDef *i2c	  = this->data->i2c;
	RingBuffer *buf		  = this->txBuf;

	if (init->I2CAckAddr != this->txAddress) {
		init->I2CAckAddr = this->txAddress;
		I2C_Cmd(i2c, DISABLE);
		I2C_Init(i2c, init);
		I2C_Cmd(i2c, ENABLE);
	}

	// TODO
	sendStop = true;

	uint8_t err	  = 0;
	bool scanOnly = false;
	if (buf->available() == 0) {
		uint32_t data = this->txAddress << 1;
		if (sendStop)
			data |= BIT_CTRL_IC_DATA_CMD_STOP;
		else
			data |= BIT_CTRL_IC_DATA_CMD_RESTART;
		i2c->IC_DATA_CMD = data | BIT_CTRL_IC_DATA_CMD_NULLDATA;

		long waitTime;
		if (this->frequency < 5000)
			waitTime = 20;
		else if (this->frequency < 10000)
			waitTime = 10;
		else
			waitTime = 5;
		delay(waitTime);

		scanOnly = true;
		goto check;
	}

	int bytesLeft;
	while (bytesLeft = buf->available()) {
		// wait for TX FIFO to be not full
		if (isFlagTimeout(i2c, this->timeout, BIT_IC_STATUS_TFNF)) {
			LT_EM(I2C, "Timeout @ 0x%02x (TX FIFO full)", this->txAddress);
			return TWOWIRE_TIMEOUT;
		}

		uint32_t data = buf->read_char();
		if (bytesLeft == 1) {
			if (sendStop)
				data |= BIT_CTRL_IC_DATA_CMD_STOP;
			else
				data |= BIT_CTRL_IC_DATA_CMD_RESTART;
		}
		i2c->IC_DATA_CMD = data;
	}

check:
	// wait for TX FIFO to be empty
	if (isFlagTimeout(i2c, this->timeout, BIT_IC_STATUS_TFE)) {
		if (!scanOnly)
			LT_EM(I2C, "Timeout @ 0x%02x (TX FIFO not empty)", this->txAddress);
		return TWOWIRE_TIMEOUT;
	}
	// check if transmission succeeded
	if (sendStop && isNoAck(i2c)) {
		if (!scanOnly)
			LT_EM(I2C, "No ACK @ 0x%02x", address);
		return scanOnly ? TWOWIRE_NACK_ADDR : TWOWIRE_NACK_DATA;
	}
	return TWOWIRE_SUCCESS;
}

size_t TwoWire::requestFrom(uint16_t address, size_t len, bool sendStop) {
	if (!this->rxBuf || this->address != 0x00)
		return 0;
	I2C_InitTypeDef *init = &this->data->init;
	I2C_TypeDef *i2c	  = this->data->i2c;
	RingBuffer *buf		  = this->rxBuf;

	if (init->I2CAckAddr != address) {
		init->I2CAckAddr = address;
		I2C_Cmd(i2c, DISABLE);
		I2C_Init(i2c, init);
		I2C_Cmd(i2c, ENABLE);
	}

	// TODO
	sendStop = true;

	int bytesLeft = len;
	buf->clear();
	while (bytesLeft && buf->availableForStore()) {
		uint32_t data = BIT_CTRL_IC_DATA_CMD_CMD;
		if (bytesLeft == 1) {
			if (sendStop)
				data |= BIT_CTRL_IC_DATA_CMD_STOP;
			else
				data |= BIT_CTRL_IC_DATA_CMD_RESTART;
		}
		i2c->IC_DATA_CMD = data;

		// check if transmission succeeded
		if (isNoAck(i2c)) {
			LT_EM(I2C, "No ACK @ 0x%02x", address);
			goto end;
		}

		// wait for RX FIFO to be not empty
		if (isFlagTimeout(i2c, timeout, BIT_IC_STATUS_RFNE)) {
			LT_EM(I2C, "Timeout @ 0x%02x (RX FIFO empty)", address);
			goto end;
		}

		buf->store_char((uint8_t)i2c->IC_DATA_CMD);

		bytesLeft--;
	}

end:
	return len - bytesLeft;
}
