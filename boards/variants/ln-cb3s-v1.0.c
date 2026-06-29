/* This file was auto-generated from ln-cb3s-v1.0.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA00, ADC2, I2C0_SCL, I2C0_SDA
	{PA_0,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C,           PIN_NONE, 0},
	// D1: PA01, ADC3, SWDIO, I2C0_SCL, I2C0_SDA
	{PA_1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_SWD, PIN_NONE, 0},
	// D2: PA04, ADC4, SWCLK, I2C0_SCL, I2C0_SDA
	{PA_4,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C | PIN_SWD, PIN_NONE, 0},
	// D3: PA05, I2C0_SCL, I2C0_SDA
	{PA_5,  PIN_GPIO | PIN_IRQ | PIN_I2C,                     PIN_NONE, 0},
	// D4: PA06, SD_D2, I2C0_SCL, I2C0_SDA
	{PA_6,  PIN_GPIO | PIN_IRQ | PIN_I2C,                     PIN_NONE, 0},
	// D5: PB04, ADC6, I2C0_SCL, I2C0_SDA
	{PB_4,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C,           PIN_NONE, 0},
	// D6: PB09, UART1_TX, I2C0_SCL, I2C0_SDA
	{PB_9,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,          PIN_NONE, 0},
	// D7: PA09, BOOT1, SD_CLK, I2S0_SCLK, I2C0_SCL, I2C0_SDA
	{PA_9,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S,           PIN_NONE, 0},
	// D8: PB05, ADC7, I2C0_SCL, I2C0_SDA
	{PB_5,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2C,           PIN_NONE, 0},
	// D9: PB06, I2C0_SCL, I2C0_SDA
	{PB_6,  PIN_GPIO | PIN_IRQ | PIN_I2C,                     PIN_NONE, 0},
	// D10: PA03, UART0_RX, I2C0_SCL, I2C0_SDA
	{PA_3,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,          PIN_NONE, 0},
	// D11: PA02, UART0_TX, I2C0_SCL, I2C0_SDA
	{PA_2,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,          PIN_NONE, 0},
	// D12: PA11, SD_D1, I2C0_SCL, I2C0_SDA
	{PA_11, PIN_GPIO | PIN_IRQ | PIN_I2C,                     PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[0]),  // PA_0 (D0)
	[1]  = &(lt_arduino_pin_info_list[1]),  // PA_1 (D1)
	[2]  = &(lt_arduino_pin_info_list[11]), // PA_2 (D11)
	[3]  = &(lt_arduino_pin_info_list[10]), // PA_3 (D10)
	[4]  = &(lt_arduino_pin_info_list[2]),  // PA_4 (D2)
	[5]  = &(lt_arduino_pin_info_list[3]),  // PA_5 (D3)
	[6]  = &(lt_arduino_pin_info_list[4]),  // PA_6 (D4)
	[9]  = &(lt_arduino_pin_info_list[7]),  // PA_9 (D7)
	[11] = &(lt_arduino_pin_info_list[12]), // PA_11 (D12)
	[20] = &(lt_arduino_pin_info_list[5]),  // PB_4 (D5)
	[21] = &(lt_arduino_pin_info_list[8]),  // PB_5 (D8)
	[22] = &(lt_arduino_pin_info_list[9]),  // PB_6 (D9)
	[25] = &(lt_arduino_pin_info_list[6]),  // PB_9 (D6)
};
// clang-format on
