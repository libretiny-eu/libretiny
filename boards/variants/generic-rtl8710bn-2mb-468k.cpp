/* This file was auto-generated from generic-rtl8710bn-2mb-468k.json using boardgen */

#include <Arduino.h>

extern "C" {

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo pinTable[PINS_COUNT] = {
	// D0: PA00, PWM2
	{PA_0,  PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D1: PA05, PWM4, WAKE1
	{PA_5,  PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D2: PA06, FCS, SD_D2
	{PA_6,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D3: PA07, FD1, SD_D3
	{PA_7,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D4: PA08, FD2, SD_CMD
	{PA_8,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D5: PA09, FD0, SD_CLK
	{PA_9,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D6: PA10, FSCK, SD_D0
	{PA_10, PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D7: PA11, FD3, SD_D1
	{PA_11, PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D8: PA12, PWM3
	{PA_12, PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D9: PA14, PWM0, SWCLK
	{PA_14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D10: PA15, PWM1, SWDIO
	{PA_15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D11: PA18, UART0_RX, SPI0_SCK, SPI1_SCK, I2C1_SCL, SD_D2, TMR4_TRIG, I2S0_MCK, WAKE0
	{PA_18, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// D12: PA19, ADC1, UART0_CTS, SPI0_CS, SPI1_CS, I2C0_SDA, SD_D3, TMR5_TRIG, I2S0_TX
	{PA_19, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D13: PA22, UART0_RTS, SPI0_MISO, SPI1_MISO, I2C0_SCL, SD_D0, PWM5, I2S0_WS, WAKE2
	{PA_22, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D14: PA23, UART0_TX, SPI0_MOSI, SPI1_MOSI, I2C1_SDA, SD_D1, PWM0, WAKE3
	{PA_23, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// D15: PA29, UART2_RX, I2C0_SCL, PWM4
	{PA_29, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// D16: PA30, UART2_TX, I2C0_SDA, PWM4
	{PA_30, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// A1: ADC2
	{AD_2,  PIN_ADC,                                                               PIN_NONE, 0},
};
// clang-format on

} // extern "C"
