/* This file was auto-generated from xh-wb3s.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P28, ADC4
	{GPIO28, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_JTAG, PIN_NONE, 0},
	// D1: P23
	{GPIO23, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D2: P14, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D3: P26, PWM5, ADC1, I2C2_SDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C,  PIN_NONE, 0},
	// D4: P24, PWM4, ADC2, I2C2_SCL
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C,  PIN_NONE, 0},
	// D5: P6, PWM0, TCK
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,           PIN_NONE, 0},
	// D6: P9, PWM3, TDO
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,           PIN_NONE, 0},
	// D7: P0, UART2_TX
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D8: P1, UART2_RX, ADC5
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,           PIN_NONE, 0},
	// D9: P8, PWM2, TDI
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,           PIN_NONE, 0},
	// D10: P10, UART1_RX, ADC6
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,           PIN_NONE, 0},
	// D11: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D12: P16, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D13: P20, ADC3
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_ADC,                      PIN_NONE, 0},
	// D14: P21
	{GPIO21, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D15: P22
	{GPIO22, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D16: P15, CS, I2C2_SCL
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,            PIN_NONE, 0},
	// D17: P17, MISO, I2C2_SDA
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,            PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[7]),  // GPIO0 (D7)
	[1]  = &(lt_arduino_pin_info_list[8]),  // GPIO1 (D8)
	[6]  = &(lt_arduino_pin_info_list[5]),  // GPIO6 (D5)
	[7]  = &(lt_arduino_pin_info_list[0]),  // GPIO28 (D0)
	[8]  = &(lt_arduino_pin_info_list[9]),  // GPIO8 (D9)
	[9]  = &(lt_arduino_pin_info_list[6]),  // GPIO9 (D6)
	[10] = &(lt_arduino_pin_info_list[10]), // GPIO10 (D10)
	[11] = &(lt_arduino_pin_info_list[11]), // GPIO11 (D11)
	[14] = &(lt_arduino_pin_info_list[2]),  // GPIO14 (D2)
	[15] = &(lt_arduino_pin_info_list[16]), // GPIO15 (D16)
	[16] = &(lt_arduino_pin_info_list[12]), // GPIO16 (D12)
	[17] = &(lt_arduino_pin_info_list[17]), // GPIO17 (D17)
	[20] = &(lt_arduino_pin_info_list[13]), // GPIO20 (D13)
	[21] = &(lt_arduino_pin_info_list[14]), // GPIO21 (D14)
	[22] = &(lt_arduino_pin_info_list[15]), // GPIO22 (D15)
	[23] = &(lt_arduino_pin_info_list[1]),  // GPIO23 (D1)
	[24] = &(lt_arduino_pin_info_list[4]),  // GPIO24 (D4)
	[26] = &(lt_arduino_pin_info_list[3]),  // GPIO26 (D3)
	[28] = &(lt_arduino_pin_info_list[0]),  // GPIO28 (D0)
};
// clang-format on
