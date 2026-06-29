/* This file was auto-generated from cb3l.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D1: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D2: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D3: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D4: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D5: P9, PWM3
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D6: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D7: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
	// D8: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D9: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D10: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D11: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[6]),  // GPIO0 (D6)
	[6]  = &(lt_arduino_pin_info_list[4]),  // GPIO6 (D4)
	[7]  = &(lt_arduino_pin_info_list[9]),  // GPIO7 (D9)
	[8]  = &(lt_arduino_pin_info_list[8]),  // GPIO8 (D8)
	[9]  = &(lt_arduino_pin_info_list[5]),  // GPIO9 (D5)
	[10] = &(lt_arduino_pin_info_list[10]), // GPIO10 (D10)
	[11] = &(lt_arduino_pin_info_list[11]), // GPIO11 (D11)
	[14] = &(lt_arduino_pin_info_list[1]),  // GPIO14 (D1)
	[21] = &(lt_arduino_pin_info_list[7]),  // GPIO21 (D7)
	[23] = &(lt_arduino_pin_info_list[0]),  // GPIO23 (D0)
	[24] = &(lt_arduino_pin_info_list[3]),  // GPIO24 (D3)
	[26] = &(lt_arduino_pin_info_list[2]),  // GPIO26 (D2)
};
// clang-format on
