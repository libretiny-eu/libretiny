/* This file was auto-generated from generic-ln882hki.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA00, ADC2, I2C0_SCL, I2C0_SDA
	{PA_0,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D1: PA01, ADC3, SWDIO, I2C0_SCL, I2C0_SDA
	{PA_1,  PIN_SWD | PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO, PIN_NONE, 0},
	// D2: PA02, UART0_TX, I2C0_SCL, I2C0_SDA
	{PA_2,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D3: PA03, UART0_RX, I2C0_SCL, I2C0_SDA
	{PA_3,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D4: PA04, ADC4, SWCLK, I2C0_SCL, I2C0_SDA
	{PA_4,  PIN_SWD | PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO, PIN_NONE, 0},
	// D5: PA05, I2C0_SCL, I2C0_SDA
	{PA_5,  PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D6: PA06, SD_D2, I2C0_SCL, I2C0_SDA
	{PA_6,  PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D7: PA07, SD_D3, I2S0_RX, I2C0_SCL, I2C0_SDA
	{PA_7,  PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D8: PA08, BOOT0, SD_CMD, I2S0_WS, I2C0_SCL, I2C0_SDA
	{PA_8,  PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D9: PA09, BOOT1, SD_CLK, I2S0_SCLK, I2C0_SCL, I2C0_SDA
	{PA_9,  PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D10: PA10, SD_D0, I2S0_TX, I2C0_SCL, I2C0_SDA
	{PA_10, PIN_I2S | PIN_I2C | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D11: PA11, SD_D1, I2C0_SCL, I2C0_SDA
	{PA_11, PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D12: PA12, I2C0_SCL, I2C0_SDA
	{PA_12, PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D13: PB03, ADC5, I2C0_SCL, I2C0_SDA
	{PB_3,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D14: PB04, ADC6, I2C0_SCL, I2C0_SDA
	{PB_4,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D15: PB05, ADC7, I2C0_SCL, I2C0_SDA
	{PB_5,  PIN_I2C | PIN_ADC | PIN_IRQ | PIN_GPIO,           PIN_NONE, 0},
	// D16: PB06, I2C0_SCL, I2C0_SDA
	{PB_6,  PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D17: PB07, I2C0_SCL, I2C0_SDA
	{PB_7,  PIN_I2C | PIN_IRQ | PIN_GPIO,                     PIN_NONE, 0},
	// D18: PB08, UART1_RX, I2C0_SCL, I2C0_SDA
	{PB_8,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
	// D19: PB09, UART1_TX, I2C0_SCL, I2C0_SDA
	{PB_9,  PIN_UART | PIN_I2C | PIN_IRQ | PIN_GPIO,          PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[0]),  // PA_0 (D0)
	[1]  = &(lt_arduino_pin_info_list[1]),  // PA_1 (D1)
	[2]  = &(lt_arduino_pin_info_list[2]),  // PA_2 (D2)
	[3]  = &(lt_arduino_pin_info_list[3]),  // PA_3 (D3)
	[4]  = &(lt_arduino_pin_info_list[4]),  // PA_4 (D4)
	[5]  = &(lt_arduino_pin_info_list[5]),  // PA_5 (D5)
	[6]  = &(lt_arduino_pin_info_list[6]),  // PA_6 (D6)
	[7]  = &(lt_arduino_pin_info_list[7]),  // PA_7 (D7)
	[8]  = &(lt_arduino_pin_info_list[8]),  // PA_8 (D8)
	[9]  = &(lt_arduino_pin_info_list[9]),  // PA_9 (D9)
	[10] = &(lt_arduino_pin_info_list[10]), // PA_10 (D10)
	[11] = &(lt_arduino_pin_info_list[11]), // PA_11 (D11)
	[12] = &(lt_arduino_pin_info_list[12]), // PA_12 (D12)
	[19] = &(lt_arduino_pin_info_list[13]), // PB_3 (D13)
	[20] = &(lt_arduino_pin_info_list[14]), // PB_4 (D14)
	[21] = &(lt_arduino_pin_info_list[15]), // PB_5 (D15)
	[22] = &(lt_arduino_pin_info_list[16]), // PB_6 (D16)
	[23] = &(lt_arduino_pin_info_list[17]), // PB_7 (D17)
	[24] = &(lt_arduino_pin_info_list[18]), // PB_8 (D18)
	[25] = &(lt_arduino_pin_info_list[19]), // PB_9 (D19)
};
// clang-format on
