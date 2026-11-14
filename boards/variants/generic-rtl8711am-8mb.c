/* Only expose pins required for log UART and the established appliance UART. */
#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	{PA_6, PIN_GPIO | PIN_UART, PIN_NONE, 0},
	{PA_7, PIN_GPIO | PIN_UART, PIN_NONE, 0},
	{PB_0, PIN_GPIO | PIN_UART, PIN_NONE, 0},
	{PB_1, PIN_GPIO | PIN_UART, PIN_NONE, 0},
	{PC_2, PIN_GPIO,			PIN_NONE, 0},
	{PC_3, PIN_GPIO,			PIN_NONE, 0},
	{PC_0, PIN_GPIO,			PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[PINS_GPIO_MAX + 1] = {
	[6]	 = &lt_arduino_pin_info_list[0],
	[7]	 = &lt_arduino_pin_info_list[1],
	[16] = &lt_arduino_pin_info_list[2],
	[17] = &lt_arduino_pin_info_list[3],
	[34] = &lt_arduino_pin_info_list[4],
	[35] = &lt_arduino_pin_info_list[5],
	[32] = &lt_arduino_pin_info_list[6],
};
