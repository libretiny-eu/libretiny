/* This file was auto-generated from cb3se.json using boardgen */

#include <Arduino.h>

extern "C" {

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo pinTable[PINS_COUNT] = {
	// D0: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG, PIN_NONE, 0},
	// D1: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
	// D2: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D3: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D4: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D5: P9, PWM3
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D6: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART, PIN_NONE, 0},
	// D7: P1, UART2_RX, I2C2_SDA
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART, PIN_NONE, 0},
	// D8: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D9: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D10: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D11: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D12: P15, SD_CMD, CS
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
	// D13: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,           PIN_NONE, 0},
	// D14: P20, I2C1_SCL, TCK, FSCK
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG, PIN_NONE, 0},
	// D15: P17, SD_D1, MISO
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
	// D16: P16, SD_D0, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
};
// clang-format on

} // extern "C"
