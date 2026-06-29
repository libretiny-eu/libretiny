/* This file was auto-generated from cr3l.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA20, SD_D1, SPI0_MISO, UART2_RTS, I2C0_SDA, PWM0
	{PIN_A20, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D1: PA02, TDO, UART1_RX, SPI0_CS, I2C0_SCL, PWM2
	{PIN_A2,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_JTAG | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D2: PA03, TDI, UART1_TX, SPI0_SCK, I2C0_SDA, PWM3
	{PIN_A3,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_JTAG | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D3: PA04, tRST, UART1_CTS, SPI0_MOSI, PWM4
	{PIN_A4,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// D4: PA15, SD_D2, SPI0_CS, UART2_RX, I2C0_SCL, PWM3
	{PIN_A15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D5: PA16, SD_D3, SPI0_SCK, UART2_TX, I2C0_SDA, PWM4
	{PIN_A16, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D6: PA17, SD_CMD, PWM5
	{PIN_A17, PIN_GPIO | PIN_IRQ | PIN_PWM,                                           PIN_NONE, 0},
	// D7: PA18, SD_CLK, PWM6
	{PIN_A18, PIN_GPIO | PIN_IRQ | PIN_PWM,                                           PIN_NONE, 0},
	// D8: PA19, SD_D0, SPI0_MOSI, UART2_CTS, I2C0_SCL, PWM7
	{PIN_A19, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D9: PA13, UART0_RX, PWM7
	{PIN_A13, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_UART,                                PIN_NONE, 0},
	// D10: PA14, SD_INT, UART0_TX, PWM2
	{PIN_A14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_UART,                                PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[2]  = &(lt_arduino_pin_info_list[1]),  // PIN_A2 (D1)
	[3]  = &(lt_arduino_pin_info_list[2]),  // PIN_A3 (D2)
	[4]  = &(lt_arduino_pin_info_list[3]),  // PIN_A4 (D3)
	[13] = &(lt_arduino_pin_info_list[9]),  // PIN_A13 (D9)
	[14] = &(lt_arduino_pin_info_list[10]), // PIN_A14 (D10)
	[15] = &(lt_arduino_pin_info_list[4]),  // PIN_A15 (D4)
	[16] = &(lt_arduino_pin_info_list[5]),  // PIN_A16 (D5)
	[17] = &(lt_arduino_pin_info_list[6]),  // PIN_A17 (D6)
	[18] = &(lt_arduino_pin_info_list[7]),  // PIN_A18 (D7)
	[19] = &(lt_arduino_pin_info_list[8]),  // PIN_A19 (D8)
	[20] = &(lt_arduino_pin_info_list[0]),  // PIN_A20 (D0)
};
// clang-format on
