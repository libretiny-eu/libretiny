/* This file was auto-generated from generic-rtl8710bl-1mb-468k.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D2: PA06, FCS
	{PA_6,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D3: PA07, FD1
	{PA_7,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D5: PA09, FD0
	{PA_9,  PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D6: PA10, FSCK
	{PA_10, PIN_GPIO | PIN_IRQ | PIN_SPI,                                          PIN_NONE, 0},
	// D9: PA14, PWM0, SWCLK
	{PA_14, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D10: PA15, PWM1, SWDIO
	{PA_15, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SWD,                                PIN_NONE, 0},
	// D11: PA18, UART0_RX, I2C1_SCL, TMR4_TRIG, WAKE0
	{PA_18, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,                               PIN_NONE, 0},
	// D14: PA23, UART0_TX, I2C1_SDA, PWM0, WAKE3
	{PA_23, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// D15: PA29, UART2_RX, I2C0_SCL, PWM4
	{PA_29, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
	// D16: PA30, UART2_TX, I2C0_SDA, PWM3
	{PA_30, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C | PIN_UART,                     PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[6]  = &(lt_arduino_pin_info_list[0]), // PA_6 (D2)
	[7]  = &(lt_arduino_pin_info_list[1]), // PA_7 (D3)
	[9]  = &(lt_arduino_pin_info_list[2]), // PA_9 (D5)
	[10] = &(lt_arduino_pin_info_list[3]), // PA_10 (D6)
	[14] = &(lt_arduino_pin_info_list[4]), // PA_14 (D9)
	[15] = &(lt_arduino_pin_info_list[5]), // PA_15 (D10)
	[18] = &(lt_arduino_pin_info_list[6]), // PA_18 (D11)
	[23] = &(lt_arduino_pin_info_list[7]), // PA_23 (D14)
	[29] = &(lt_arduino_pin_info_list[8]), // PA_29 (D15)
	[30] = &(lt_arduino_pin_info_list[9]), // PA_30 (D16)
};
// clang-format on
