/* Copyright (c) Kuba Szczodrzyński 2023-09-23. */

#include "SPIPrivate.h"

bool SPIClass::beginPrivate() {
	if (!this->data)
		return false;

	this->data->spi = SPI_DEV_TABLE[this->port].SPIx;
	this->data->irq = (IRQn)SPI_DEV_TABLE[this->port].IrqNum;

	switch (this->port) {
		case 0:
			RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);
			break;
		case 1:
			RCC_PeriphClockCmd(APBPeriph_SPI1, APBPeriph_SPI1_CLOCK, ENABLE);
			break;
	}

	Pinmux_Config(this->sck, PINMUX_FUNCTION_SPIM);
	Pinmux_Config(this->miso, PINMUX_FUNCTION_SPIM);
	Pinmux_Config(this->mosi, PINMUX_FUNCTION_SPIM);

	SSI_InitTypeDef *init = &this->data->init;
	SPI_TypeDef *spi	  = this->data->spi;
	IRQn irq			  = this->data->irq;
	SSI_StructInit(init);
	init->SPI_Role = SSI_MASTER;

	SSI_Cmd(spi, DISABLE);
	SSI_Init(spi, init);
	SSI_Cmd(spi, ENABLE);

	// VECTOR_IrqUnRegister(irq);
	// VECTOR_IrqRegister((IRQ_FUN)callback, irq, (u32)this->data, 10);
	// VECTOR_IrqEn(irq, 10);

	return true;
}

bool SPIClass::endPrivate() {
	if (!this->data)
		return true;
	SPI_TypeDef *spi = this->data->spi;
	IRQn irq		 = this->data->irq;

	SSI_INTConfig(spi, BIT_IMR_RXFIM | BIT_IMR_RXOIM | BIT_IMR_RXUIM, DISABLE);
	VECTOR_IrqDis(irq);
	VECTOR_IrqUnRegister(irq);
	SSI_Cmd(spi, DISABLE);

	return true;
}

void SPIClass::setFrequency(uint32_t frequency) {
	if (!this->data)
		return;
	uint32_t maxFrequency = CPU_ClkGet(false) >> 2;
	if (frequency > maxFrequency) {
		LT_WM(SPI, "Clock freq too high! %lu > %lu", frequency, maxFrequency);
		SSI_SetBaud(this->data->spi, maxFrequency, maxFrequency << 1);
	} else {

		SSI_SetBaud(this->data->spi, frequency, maxFrequency << 1);
	}
	this->settings._clock = frequency;
}

void SPIClass::setClockDivider(uint32_t clockDiv) {
	if (!this->data)
		return;
	if (clockDiv < 4) {
		LT_EM(SPI, "Clock div too low! %lu < 4", clockDiv);
		return;
	}
	SSI_SetBaudDiv(this->data->spi, clockDiv << 1);
	this->settings._clock = CPU_ClkGet(false) / clockDiv;
}

void SPIClass::setDataMode(uint8_t dataMode) {
	if (!this->data)
		return;
	this->settings._dataMode = dataMode;
	SSI_SetSclkPolarity(this->data->spi, (dataMode >> 1) & 0b1);
	SSI_SetSclkPhase(this->data->spi, (dataMode >> 0) & 0b1);
}

uint8_t SPIClass::transfer(uint8_t data) {
	while ((SPI1_DEV->SR & BIT_SR_TFNF) == 0) {}
	SPI1_DEV->DR[0] = data;
	while ((SPI1_DEV->SR & BIT_SR_RFNE) == 0) {}
	return SPI1_DEV->DR[0];
}

void SPIClass::write(uint8_t data) {
	while ((SPI1_DEV->SR & BIT_SR_TFNF) == 0) {}
	SPI1_DEV->DR[0] = data;
}
