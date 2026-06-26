/* WF200 SPI proof-of-life probe — WRONG BUS for the WGM160P; kept as a
 * negative result + template.
 *
 * This probe assumed the in-package WF200 sat on USART3-LOC0 SPI
 * (PA0-3). It reads 0xFFFFFFFF on a known-working unit because the
 * WGM160P actually connects its WF200 over **SDIO**: DAT0-3 = PA0-PA3,
 * CLK = PE14, CMD = PE15, all LOC1 (see the SD-card-slot route in the
 * GSDK's wfx_host/sdio/bsp_sd_hc.c and the WGM160PX22KGA2 block in
 * sl_wfx_host_init.c). PA0-3 carry SDIO data lines, which a USART
 * can't speak. A real proof-of-life needs the GG11 SDIO peripheral
 * issuing CMD0/CMD8/CMD5 — Phase 2 work, not a register-poke probe.
 *
 * Control pins (confirmed against GSDK brd4321a config): PF12=RESETn,
 * PE4=WUP, PA8=WIRQ, PA12=LP_CLK.
 *
 * Results land in `wf200_probe` (read over SWD); red LED = non-FF
 * response (would indicate something SPI-ish actually answered),
 * blue = no response (expected on a WGM160P).
 */

#include <Arduino.h>

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

// [0] done-magic 0xCAFE0001, [1..3] CONFIG readbacks of 3 attempts
volatile uint32_t wf200_probe[4];

static uint32_t wf200_read_config(void) {
	uint32_t value = 0;
	GPIO_PinOutClear(gpioPortA, 3); // CS low
	// header: SET_READ(0x8000) | (CONFIG_REG_ID 0 << 12) | word_count(2), big-endian
	USART_SpiTransfer(USART3, 0x80);
	USART_SpiTransfer(USART3, 0x02);
	for (int i = 0; i < 4; i++)
		value = (value << 8) | USART_SpiTransfer(USART3, 0xFF);
	GPIO_PinOutSet(gpioPortA, 3); // CS high
	return value;
}

void setup() {
	CMU_ClockEnable(cmuClock_USART3, true);

	// WF200 control lines
	GPIO_PinModeSet(gpioPortE, 4, gpioModePushPull, 1);	 // WUP high
	GPIO_PinModeSet(gpioPortF, 12, gpioModePushPull, 0); // RESETn asserted
	// SPI lines (USART3 LOC0)
	GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0); // MOSI
	GPIO_PinModeSet(gpioPortA, 1, gpioModeInput, 0);	// MISO
	GPIO_PinModeSet(gpioPortA, 2, gpioModePushPull, 0); // SCK
	GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 1); // CS idle high

	USART_InitSync_TypeDef spiInit = USART_INITSYNC_DEFAULT;
	spiInit.baudrate			   = 1000000; // 1 MHz probe speed
	spiInit.msbf				   = true;	  // SPI mode 0, MSB first
	USART_InitSync(USART3, &spiInit);
	USART3->ROUTELOC0 = USART_ROUTELOC0_TXLOC_LOC0 | USART_ROUTELOC0_RXLOC_LOC0 | USART_ROUTELOC0_CLKLOC_LOC0;
	USART3->ROUTEPEN  = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN;

	// Reset pulse, then give the WF200 bootloader time to come up
	delay(10);
	GPIO_PinOutSet(gpioPortF, 12); // release RESETn
	delay(50);

	for (int i = 0; i < 3; i++) {
		wf200_probe[1 + i] = wf200_read_config();
		delay(10);
	}
	wf200_probe[0] = 0xCAFE0001;

	uint32_t v	= wf200_probe[1];
	bool alive	= (v != 0x00000000UL) && (v != 0xFFFFFFFFUL);
	uint8_t led = alive ? LED_R : LED_B;
	pinMode(led, OUTPUT);
	digitalWrite(led, HIGH);
}

void loop() {
	delay(1000);
}
