/* This file was auto-generated from t1-2s.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P26, PWM5, ADC1, I2C2_SDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D1: P6, PWM0, TCK
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D2: P8, PWM2, TDI
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D3: P1, UART2_RX, ADC5
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D4: P10, UART1_RX, ADC6
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D5: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
	// D6: P9, PWM3, TDO
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D7: P24, PWM4, ADC2, I2C2_SCL
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D11: P0, UART2_TX
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[8]), // GPIO0 (D11)
	[1]  = &(lt_arduino_pin_info_list[3]), // GPIO1 (D3)
	[6]  = &(lt_arduino_pin_info_list[1]), // GPIO6 (D1)
	[8]  = &(lt_arduino_pin_info_list[2]), // GPIO8 (D2)
	[9]  = &(lt_arduino_pin_info_list[6]), // GPIO9 (D6)
	[10] = &(lt_arduino_pin_info_list[4]), // GPIO10 (D4)
	[11] = &(lt_arduino_pin_info_list[5]), // GPIO11 (D5)
	[24] = &(lt_arduino_pin_info_list[7]), // GPIO24 (D7)
	[26] = &(lt_arduino_pin_info_list[0]), // GPIO26 (D0)
};
// clang-format on
