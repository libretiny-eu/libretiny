/* This file was auto-generated from bw15.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA17, SD_CMD, PWM5
	{PIN_A17, PIN_GPIO | PIN_IRQ | PIN_PWM,                                           PIN_NONE, 0},
	// D1: PA18, SD_CLK, PWM6
	{PIN_A18, PIN_GPIO | PIN_IRQ | PIN_PWM,                                           PIN_NONE, 0},
	// D2: PA02, TDO, UART1_RX, SPI0_CS, I2C0_SCL, PWM2
	{PIN_A2,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_JTAG | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D3: PA15, SD_D2, SPI0_CS, UART2_RX, I2C0_SCL, PWM3
	{PIN_A15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D4: PA04, tRST, UART1_CTS, SPI0_MOSI, PWM4
	{PIN_A4,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SPI | PIN_UART,           PIN_NONE, 0},
	// D5: PA19, SD_D0, SPI0_MOSI, UART2_CTS, I2C0_SCL, PWM7
	{PIN_A19, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D6: PA20, SD_D1, SPI0_MISO, UART2_RTS, I2C0_SDA, PWM0
	{PIN_A20, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D7: PA16, SD_D3, SPI0_SCK, UART2_TX, I2C0_SDA, PWM4
	{PIN_A16, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_SPI | PIN_UART,            PIN_NONE, 0},
	// D8: PA00, TCK, UART1_RX, PWM0, SWCLK
	{PIN_A0,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SWD | PIN_UART,           PIN_NONE, 0},
	// D9: PA03, TDI, UART1_TX, SPI0_SCK, I2C0_SDA, PWM3
	{PIN_A3,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_JTAG | PIN_SPI | PIN_UART, PIN_NONE, 0},
	// D10: PA01, TMS, UART1_TX, PWM1, SWDIO
	{PIN_A1,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG | PIN_SWD | PIN_UART,           PIN_NONE, 0},
	// D11: PA13, UART0_RX, PWM7
	{PIN_A13, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_UART,                                PIN_NONE, 0},
	// D12: PA14, SD_INT, UART0_TX, PWM2
	{PIN_A14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_UART,                                PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[8]),  // PIN_A0 (D8)
	[1]  = &(lt_arduino_pin_info_list[10]), // PIN_A1 (D10)
	[2]  = &(lt_arduino_pin_info_list[2]),  // PIN_A2 (D2)
	[3]  = &(lt_arduino_pin_info_list[9]),  // PIN_A3 (D9)
	[4]  = &(lt_arduino_pin_info_list[4]),  // PIN_A4 (D4)
	[13] = &(lt_arduino_pin_info_list[11]), // PIN_A13 (D11)
	[14] = &(lt_arduino_pin_info_list[12]), // PIN_A14 (D12)
	[15] = &(lt_arduino_pin_info_list[3]),  // PIN_A15 (D3)
	[16] = &(lt_arduino_pin_info_list[7]),  // PIN_A16 (D7)
	[17] = &(lt_arduino_pin_info_list[0]),  // PIN_A17 (D0)
	[18] = &(lt_arduino_pin_info_list[1]),  // PIN_A18 (D1)
	[19] = &(lt_arduino_pin_info_list[5]),  // PIN_A19 (D5)
	[20] = &(lt_arduino_pin_info_list[6]),  // PIN_A20 (D6)
};
// clang-format on
