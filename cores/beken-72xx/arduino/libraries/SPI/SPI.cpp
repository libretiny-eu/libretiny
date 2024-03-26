/* Copyright (c) Kuba Szczodrzyński 2024-03-15. */

#include "SPIPrivate.h"

#define SPI_PERI_CLK_26M (26 * 1000 * 1000)
#define SPI_PERI_CLK_DCO (80 * 1000 * 1000)

bool SPIClass::beginPrivate() {
	if (!this->data)
		return false;
	uint32_t param;

	REG_SPI0->ctrl = 0;

	intc_service_change_handler(IRQ_SPI, (FUNCPTR)SPIClass::isrHandlerStatic);
	// bk_spi_configure()
	REG_SPI0->BIT_WIDTH = 0;
	REG_SPI0->MSTEN		= true;
	// disable hardware CS - issues with per-byte data transfer otherwise
#if LT_BK7231N
	REG_SPI0->WIRE3_EN = true;
#else
	REG_SPI0->NSSMD = 3;
#endif
	// spi_init_msten(0)
	REG_SPI0->TX_FIFO_INT_LEVEL = 0;
	REG_SPI0->TX_FIFO_INT_LEVEL = 0;
	REG_SPI0->TX_UDF_INT_EN		= true;
	REG_SPI0->RX_OVF_INT_EN		= true;
	// spi_active(1)
	REG_SPI0->SPIEN = true;
	// spi_icu_configuration(1)
	param = PWD_SPI_CLK_BIT;
	icu_ctrl(CMD_CLK_PWR_UP, &param);
	param = IRQ_SPI_BIT;
	icu_ctrl(CMD_ICU_INT_ENABLE, &param);
	// spi_gpio_configuration()
	param = GFUNC_MODE_SPI;
	gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);

	return true;
}

bool SPIClass::endPrivate() {
	if (!this->data)
		return true;
	uint32_t param;

	param = IRQ_SPI_BIT;
	icu_ctrl(CMD_ICU_INT_DISABLE, &param);
	param = PWD_SPI_CLK_BIT;
	icu_ctrl(CMD_CLK_PWR_DOWN, &param);

	REG_SPI0->ctrl = 0;

	return true;
}

void SPIClass::setFrequency(uint32_t frequency) {
	if (!this->data)
		return;
	uint32_t param;
	uint32_t maxFrequency = 30000000;
	uint32_t sourceClk;
	uint32_t clockDiv;

	REG_SPI0->SPIEN		  = false;
	this->settings._clock = frequency;

	if (frequency == 26000000 || frequency == 13000000 || frequency == 6500000 || frequency <= 4333000) {
#if CFG_XTAL_FREQUENCE
		sourceClk = CFG_XTAL_FREQUENCE;
#else
		sourceClk = SPI_PERI_CLK_26M;
#endif
		param = PCLK_POSI_SPI;
		icu_ctrl(CMD_CONF_PCLK_26M, &param);
	} else {
		if (frequency > maxFrequency) {
			LT_WM(SPI, "Clock freq too high! %lu > %lu", frequency, maxFrequency);
			frequency = maxFrequency;
		}
		sourceClk = SPI_PERI_CLK_DCO;
		param	  = PWD_SPI_CLK_BIT;
		icu_ctrl(CMD_CLK_PWR_DOWN, &param);
		param = PCLK_POSI_SPI;
		icu_ctrl(CMD_CONF_PCLK_DCO, &param);
		param = PWD_SPI_CLK_BIT;
		icu_ctrl(CMD_CLK_PWR_UP, &param);
	}

	if (frequency == 26000000 || frequency == 13000000 || frequency == 6500000) {
		clockDiv = sourceClk / frequency - 1;
	} else {
		clockDiv = ((sourceClk >> 1) / frequency);
		if (clockDiv < 2)
			clockDiv = 2;
		else if (clockDiv >= 255)
			clockDiv = 255;
	}

	REG_SPI0->SPI_CKR = clockDiv;
	REG_SPI0->SPIEN	  = true;
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
	if (!this->data)
		return;
	this->settings._bitOrder = bitOrder;
#if LT_BK7231N
	REG_SPI0->LSB_FIRST = bitOrder == SPI_LSBFIRST;
#endif
}

void SPIClass::setDataMode(uint8_t dataMode) {
	if (!this->data)
		return;
	this->settings._dataMode = dataMode;
	REG_SPI0->CKPOL			 = (dataMode >> 1) & 0b1;
	REG_SPI0->CKPHA			 = (dataMode >> 0) & 0b1;
}

void SPIClass::commitTransaction() {
	if (this->txLen != 0) {
		REG_SPI0->TX_FIFO_CLR	 = true;
		REG_SPI0->TX_TRANS_LEN	 = this->txLen * 8;
		REG_SPI0->TX_FIFO_INT_EN = false;
		REG_SPI0->TX_EN			 = true;
		while (this->txLen--) {
			while (REG_SPI0->TX_FIFO_WR_READY == false) {}
			uint32_t data = *this->txBuf++;
			// LT_I("<- TX: %02x", data);
			REG_SPI0->SPI_DAT = data;
		}
		REG_SPI0->TX_FIFO_INT_EN = false;
		REG_SPI0->TX_EN			 = false;
	}
	if (this->rxLen != 0) {
		// REG_SPI0->RX_FIFO_CLR	   = true;
		// REG_SPI0->RX_TRANS_LEN	   = this->rxLen * 8;
		// REG_SPI0->RX_FIFO_INT_EN   = false;
		// REG_SPI0->RX_FINISH_INT_EN = false;
		// REG_SPI0->RX_EN			   = true;
		// while (this->rxLen--) {
		// 	while (REG_SPI0->RX_FIFO_RD_READY == false) {}
		// 	uint32_t data = REG_SPI0->SPI_DAT;
		// 	// LT_I("-> RX: %02x", data);
		// 	*this->rxBuf++ = data;
		// }
		// REG_SPI0->RX_EN			 = false;
		// REG_SPI0->TX_FIFO_INT_EN = false;
		// REG_SPI0->RX_FIFO_INT_EN = false;
	}
}

void SPIClass::isrHandlerStatic(void *param) {
	SPI.isrHandler(param);
}

void SPIClass::isrHandler(void *param) {
	if (REG_SPI0->RX_FIFO_INT) {
		LT_I("RX_FIFO_INT, rxLen=%d", this->rxLen);
		while (this->rxLen != 0 && REG_SPI0->RX_FIFO_RD_READY == true) {
			uint8_t data = REG_SPI0->SPI_DAT;
			LT_I("RX data in ISR #1 %02x", data);
			*this->rxBuf++ = data;
			this->rxLen--;
		}
		if (this->rxLen == 0) {
			REG_SPI0->RX_EN			 = false;
			REG_SPI0->TX_FIFO_INT_EN = false;
			REG_SPI0->RX_FIFO_INT_EN = false;
		}
		REG_SPI0->RX_FIFO_INT = true;
	}
	if (REG_SPI0->RX_FINISH_INT) {
		LT_I("RX_FINISH_INT, rxLen=%d", this->rxLen);
		while (this->rxLen != 0 && REG_SPI0->RX_FIFO_RD_READY == true) {
			uint8_t data = REG_SPI0->SPI_DAT;
			LT_I("RX data in ISR #2 %02x", data);
			*this->rxBuf++ = data;
			this->rxLen--;
		}
		REG_SPI0->RX_EN			 = false;
		REG_SPI0->TX_FIFO_INT_EN = false;
		REG_SPI0->RX_FIFO_INT_EN = false;
		REG_SPI0->RX_FINISH_INT	 = true;
	}

	if (REG_SPI0->TX_UDF_INT) {
		LT_W("TX underflow");
		REG_SPI0->TX_UDF_INT = true;
	}
	if (REG_SPI0->RX_OVF_INT) {
		LT_W("RX overflow");
		REG_SPI0->RX_OVF_INT = true;
	}
}
