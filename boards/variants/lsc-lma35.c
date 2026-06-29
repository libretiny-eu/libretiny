/* This file was auto-generated from lsc-lma35.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D1: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D2: P16, SD_D0, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D3: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D4: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,                     PIN_NONE, 0},
	// D5: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D6: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D7: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D8: P9, PWM3
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D9: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
	// D10: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D11: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D12: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D13: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D14: P1, UART2_RX, I2C2_SDA
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[5]),  // GPIO0 (D5)
	[1]  = &(lt_arduino_pin_info_list[14]), // GPIO1 (D14)
	[6]  = &(lt_arduino_pin_info_list[10]), // GPIO6 (D10)
	[7]  = &(lt_arduino_pin_info_list[11]), // GPIO7 (D11)
	[8]  = &(lt_arduino_pin_info_list[7]),  // GPIO8 (D7)
	[9]  = &(lt_arduino_pin_info_list[8]),  // GPIO9 (D8)
	[10] = &(lt_arduino_pin_info_list[12]), // GPIO10 (D12)
	[11] = &(lt_arduino_pin_info_list[13]), // GPIO11 (D13)
	[14] = &(lt_arduino_pin_info_list[1]),  // GPIO14 (D1)
	[16] = &(lt_arduino_pin_info_list[2]),  // GPIO16 (D2)
	[21] = &(lt_arduino_pin_info_list[9]),  // GPIO21 (D9)
	[22] = &(lt_arduino_pin_info_list[4]),  // GPIO22 (D4)
	[23] = &(lt_arduino_pin_info_list[6]),  // GPIO23 (D6)
	[24] = &(lt_arduino_pin_info_list[3]),  // GPIO24 (D3)
	[26] = &(lt_arduino_pin_info_list[0]),  // GPIO26 (D0)
};
// clang-format on
