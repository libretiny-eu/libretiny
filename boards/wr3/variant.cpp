/* This file was auto-generated from wr3.json using boardgen */

#include <Arduino.h>

extern "C" {

// clang-format off
PinInfo pinTable[PINS_COUNT] = {
	// D0: PA22, UART0_RTS, SPI0_MISO, SPI1_MISO, I2C0_SCL, SD_D0, PWM5, I2S0_WS, WAKE2
	{PA_22, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D1: PA19, ADC1, UART0_CTS, SPI0_CS, SPI1_CS, I2C0_SDA, SD_D3, TMR5_TRIG, I2S0_TX
	{PA_19, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D2: PA14, PWM0, SWCLK
	{PA_14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D3: PA15, PWM1, SWDIO
	{PA_15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D4: PA00, PWM2
	{PA_0,  PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D5: PA29, UART2_RX, I2C0_SCL, PWM4
	{PA_29, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// D6: PA30, UART2_TX, I2C0_SDA, PWM4
	{PA_30, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// D7: PA05, PWM4, WAKE1
	{PA_5,  PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D8: PA12, PWM3
	{PA_12, PIN_GPIO | PIN_IRQ | PIN_PWM,                                          PIN_NONE, 0},
	// D9: PA18, UART0_RX, SPI0_SCK, SPI1_SCK, I2C1_SCL, SD_D2, TMR4_TRIG, I2S0_MCK, WAKE0
	{PA_18, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// D10: PA23, UART0_TX, SPI0_MOSI, SPI1_MOSI, I2C1_SDA, SD_D1, PWM0, WAKE3
	{PA_23, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// A1: ADC2
	{AD_2,  PIN_ADC,                                                               PIN_NONE, 0},
};
// clang-format on

} // extern "C"
