/* This file was auto-generated from t1-u.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P14, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                     PIN_NONE, 0},
	// D1: P16, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                     PIN_NONE, 0},
	// D2: P23
	{GPIO23, PIN_GPIO | PIN_IRQ,                               PIN_NONE, 0},
	// D3: P22
	{GPIO22, PIN_GPIO | PIN_IRQ,                               PIN_NONE, 0},
	// D4: P20, ADC3
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_ADC,                     PIN_NONE, 0},
	// D5: P1, UART2_RX, ADC5
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D6: P0, UART2_TX
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
	// D7: P24, PWM4, ADC2, I2C2_SCL
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D8: P9, PWM3, TDO
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D9: P26, PWM5, ADC1, I2C2_SDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_ADC | PIN_I2C, PIN_NONE, 0},
	// D10: P6, PWM0, TCK
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D11: P8, PWM2, TDI
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_JTAG,          PIN_NONE, 0},
	// D12: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                    PIN_NONE, 0},
	// D13: P10, UART1_RX, ADC6
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,          PIN_NONE, 0},
	// D14: P28, ADC4
	{GPIO28, PIN_GPIO | PIN_IRQ | PIN_ADC,                     PIN_NONE, 0},
	// D15: P21
	{GPIO21, PIN_GPIO | PIN_IRQ,                               PIN_NONE, 0},
	// D16: P17, MISO, I2C2_SDA
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,           PIN_NONE, 0},
	// D17: P15, CS, I2C2_SCL
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_SPI,           PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[6]),  // GPIO0 (D6)
	[1]  = &(lt_arduino_pin_info_list[5]),  // GPIO1 (D5)
	[6]  = &(lt_arduino_pin_info_list[10]), // GPIO6 (D10)
	[8]  = &(lt_arduino_pin_info_list[11]), // GPIO8 (D11)
	[9]  = &(lt_arduino_pin_info_list[8]),  // GPIO9 (D8)
	[10] = &(lt_arduino_pin_info_list[13]), // GPIO10 (D13)
	[11] = &(lt_arduino_pin_info_list[12]), // GPIO11 (D12)
	[14] = &(lt_arduino_pin_info_list[0]),  // GPIO14 (D0)
	[15] = &(lt_arduino_pin_info_list[17]), // GPIO15 (D17)
	[16] = &(lt_arduino_pin_info_list[1]),  // GPIO16 (D1)
	[17] = &(lt_arduino_pin_info_list[16]), // GPIO17 (D16)
	[20] = &(lt_arduino_pin_info_list[4]),  // GPIO20 (D4)
	[21] = &(lt_arduino_pin_info_list[15]), // GPIO21 (D15)
	[22] = &(lt_arduino_pin_info_list[3]),  // GPIO22 (D3)
	[23] = &(lt_arduino_pin_info_list[2]),  // GPIO23 (D2)
	[24] = &(lt_arduino_pin_info_list[7]),  // GPIO24 (D7)
	[26] = &(lt_arduino_pin_info_list[9]),  // GPIO26 (D9)
	[28] = &(lt_arduino_pin_info_list[14]), // GPIO28 (D14)
};
// clang-format on
