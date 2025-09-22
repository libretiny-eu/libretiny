/* This file was auto-generated from wl2s.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA07, SD_D3, I2S0_RX, I2C0_SCL, I2C0_SDA
	{PA_7,  PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D1: PA12, I2C0_SCL, I2C0_SDA
	{PA_12, PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D2: PA03, UART0_RX, I2C0_SCL, I2C0_SDA
	{PA_3,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D3: PA10, SD_D0, I2S0_TX, I2C0_SCL, I2C0_SDA
	{PA_10, PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D4: PA02, UART0_TX, I2C0_SCL, I2C0_SDA
	{PA_2,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D5: PA00, ADC2, I2C0_SCL, I2C0_SDA
	{PA_0,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D6: PB03, ADC5, I2C0_SCL, I2C0_SDA
	{PB_3,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D7: PA11, SD_D1, I2C0_SCL, I2C0_SDA
	{PA_11, PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D8: PA09, BOOT1, SD_CLK, I2S0_SCLK, I2C0_SCL, I2C0_SDA
	{PA_9,  PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D9: PB08, UART1_RX, I2C0_SCL, I2C0_SDA
	{PB_8,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D10: PB09, UART1_TX, I2C0_SCL, I2C0_SDA
	{PB_9,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D11: PA05, I2C0_SCL, I2C0_SDA
	{PA_5,  PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D12: PA01, ADC3, SWDIO, I2C0_SCL, I2C0_SDA
	{PA_1,  PIN_SWD | PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO, PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[5]),  // PA_0 (D5)
	[1]  = &(lt_arduino_pin_info_list[12]), // PA_1 (D12)
	[2]  = &(lt_arduino_pin_info_list[4]),  // PA_2 (D4)
	[3]  = &(lt_arduino_pin_info_list[2]),  // PA_3 (D2)
	[5]  = &(lt_arduino_pin_info_list[11]), // PA_5 (D11)
	[7]  = &(lt_arduino_pin_info_list[0]),  // PA_7 (D0)
	[9]  = &(lt_arduino_pin_info_list[8]),  // PA_9 (D8)
	[10] = &(lt_arduino_pin_info_list[3]),  // PA_10 (D3)
	[11] = &(lt_arduino_pin_info_list[7]),  // PA_11 (D7)
	[12] = &(lt_arduino_pin_info_list[1]),  // PA_12 (D1)
	[19] = &(lt_arduino_pin_info_list[6]),  // PB_3 (D6)
	[24] = &(lt_arduino_pin_info_list[9]),  // PB_8 (D9)
	[25] = &(lt_arduino_pin_info_list[10]), // PB_9 (D10)
};
// clang-format on
