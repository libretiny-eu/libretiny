/*
	Copyright (c) 2011 Arduino.  All right reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "variant.h"
#include "gpio_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "PinNames.h"

void __libc_init_array(void);

/*
 * Pins descriptions
 */
PinDescription g_APinDescription[TOTAL_GPIO_PIN_NUM] = {
	// D0: UART0_RTS, SPI1_MISO, SPI0_MISO, I2C0_SCL, SD_D0, PWM5, I2S_WS, WAKEUP_2
	{PA_22, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D1: UART0_CTS, SPI1_CS, SPI0_CS, I2C0_SDA, SD_D3, TIMER5_TRIG, I2S_SD_TX, ADC1
	{PA_19, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ,			NOT_INITIAL},
	// D2: PWM0, SWD_CLK
	{PA_14, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D3: PWM1, SWD_DATA
	{PA_15, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D4: PWM2, ext_32K
	{PA_0,  NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D5: SD_CMD, I2S_SD_RX, ADC3
	{PA_20, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ,			NOT_INITIAL},
	// D6: UART2_log_RXD, I2C0_SCL, PWM4
	{PA_29, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D7: UART2_log_TXD, I2C0_SDA, PWM3, RTC_OUT
	{PA_30, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D8: SDIO_SIDEBAND_INT, PWM4, WAKEUP_1
	{PA_5,  NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM,	NOT_INITIAL},
	// D9: PWM3
	{PA_12, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM, NOT_INITIAL},
	// D10: UART0_RXD, SPI1_CLK, SPI0_SCK, I2C1_SCL, SD_D2, TIMER4_TRIG, I2S_MCK, WAKEUP_0
	{PA_18, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ,			NOT_INITIAL},
	// D11: UART0_TXD, SPI1_MOSI, SPI0_MOSI, I2C1_SDA, SD_D1, PWM0, WAKEUP_3
	{PA_23, NOT_INITIAL, PIO_GPIO | PIO_GPIO_IRQ | PIO_PWM, NOT_INITIAL},
};

void *gpio_pin_struct[TOTAL_GPIO_PIN_NUM] = {NULL};
void *gpio_irq_handler_list[TOTAL_GPIO_PIN_NUM] = {NULL};

/** The heap API in OS layer */
// extern int vPortAddHeapRegion(uint8_t *addr, size_t size);

// it should be the last symbol in SRAM in link result
// extern void *__HeapLimit;

// it should be the last symbol in SDRAM in link result
// extern void *__sdram_bss_end__;

// extern int HalPinCtrlRtl8195A(int  Function, int PinLocation, int Operation);

#ifdef __cplusplus
} // extern C
#endif

void serialEvent() __attribute__((weak));
bool Serial_available() __attribute__((weak));

// ----------------------------------------------------------------------------

void serialEventRun(void) {
	if (Serial_available && serialEvent && Serial_available())
		serialEvent();
}

void init(void) {
	// Initialize C library
	__libc_init_array();
}

// ----------------------------------------------------------------------------

void wait_for_debug() {
	while (((CoreDebug->DHCSR) & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		asm("nop");
	}
	delay(1000);
}
