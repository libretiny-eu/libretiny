/* This file was auto-generated from wb02a.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA07, SD_D3, I2S0_RX, I2C0_SCL, I2C0_SDA
	{PA_7,  , PIN_NONE, 0},
	// D1: PA05, I2C0_SCL, I2C0_SDA
	{PA_5,  , PIN_NONE, 0},
	// D2: PA03, UART0_RX, I2C0_SCL, I2C0_SDA
	{PA_3,  , PIN_NONE, 0},
	// D3: PA10, SD_D0, I2S0_TX, I2C0_SCL, I2C0_SDA
	{PA_10, , PIN_NONE, 0},
	// D4: PA02, UART0_TX, I2C0_SCL, I2C0_SDA
	{PA_2,  , PIN_NONE, 0},
	// D5: PA01, ADC3, SWDIO, I2C0_SCL, I2C0_SDA
	{PA_1,  , PIN_NONE, 0},
	// D6: PA04, ADC4, SWCLK, I2C0_SCL, I2C0_SDA
	{PA_4,  , PIN_NONE, 0},
	// D7: PA09, BOOT1, SD_CLK, I2S0_SCLK, I2C0_SCL, I2C0_SDA
	{PA_9,  , PIN_NONE, 0},
	// D8: PB08, UART1_RX, I2C0_SCL, I2C0_SDA
	{PB_8,  , PIN_NONE, 0},
	// D9: PB09, UART1_TX, I2C0_SCL, I2C0_SDA
	{PB_9,  , PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[1]  = &(lt_arduino_pin_info_list[5]), // PA_1 (D5)
	[2]  = &(lt_arduino_pin_info_list[4]), // PA_2 (D4)
	[3]  = &(lt_arduino_pin_info_list[2]), // PA_3 (D2)
	[4]  = &(lt_arduino_pin_info_list[6]), // PA_4 (D6)
	[5]  = &(lt_arduino_pin_info_list[1]), // PA_5 (D1)
	[7]  = &(lt_arduino_pin_info_list[0]), // PA_7 (D0)
	[9]  = &(lt_arduino_pin_info_list[7]), // PA_9 (D7)
	[10] = &(lt_arduino_pin_info_list[3]), // PA_10 (D3)
	[24] = &(lt_arduino_pin_info_list[8]), // PB_8 (D8)
	[25] = &(lt_arduino_pin_info_list[9]), // PB_9 (D9)
};
// clang-format on
