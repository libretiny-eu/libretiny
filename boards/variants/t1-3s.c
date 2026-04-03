/* This file was auto-generated from t1-3s.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P20, ADC3
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_ADC,                     PIN_NONE, 0},
	// D1: P22
	{GPIO22, PIN_GPIO | PIN_IRQ,                               PIN_NONE, 0},
	// D2: P6, PWM0, TCK
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D3: P8, PWM2, TDI
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D4: P9, PWM3, TDO
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D5: P23
	{GPIO23, PIN_GPIO | PIN_IRQ,                               PIN_NONE, 0},
	// D6: P0, UART2_TX
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
	// D7: P1, UART2_RX, ADC5
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D8: P24, PWM4, ADC2, I2C2_SCL
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D9: P26, PWM5, ADC1, I2C2_SDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D10: P10, UART1_RX, ADC6
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D11: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[6]),  // GPIO0 (D6)
	[1]  = &(lt_arduino_pin_info_list[7]),  // GPIO1 (D7)
	[6]  = &(lt_arduino_pin_info_list[2]),  // GPIO6 (D2)
	[8]  = &(lt_arduino_pin_info_list[3]),  // GPIO8 (D3)
	[9]  = &(lt_arduino_pin_info_list[4]),  // GPIO9 (D4)
	[10] = &(lt_arduino_pin_info_list[10]), // GPIO10 (D10)
	[11] = &(lt_arduino_pin_info_list[11]), // GPIO11 (D11)
	[20] = &(lt_arduino_pin_info_list[0]),  // GPIO20 (D0)
	[22] = &(lt_arduino_pin_info_list[1]),  // GPIO22 (D1)
	[23] = &(lt_arduino_pin_info_list[5]),  // GPIO23 (D5)
	[24] = &(lt_arduino_pin_info_list[8]),  // GPIO24 (D8)
	[26] = &(lt_arduino_pin_info_list[9]),  // GPIO26 (D9)
};
// clang-format on
