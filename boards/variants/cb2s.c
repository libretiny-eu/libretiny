/* This file was auto-generated from cb2s.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D1: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D2: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D3: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D4: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D5: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D6: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D7: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D8: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D9: P1, UART2_RX, I2C2_SDA
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D10: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[8]),  // GPIO0 (D8)
	[1]  = &(lt_arduino_pin_info_list[9]),  // GPIO1 (D9)
	[6]  = &(lt_arduino_pin_info_list[0]),  // GPIO6 (D0)
	[7]  = &(lt_arduino_pin_info_list[1]),  // GPIO7 (D1)
	[8]  = &(lt_arduino_pin_info_list[2]),  // GPIO8 (D2)
	[10] = &(lt_arduino_pin_info_list[4]),  // GPIO10 (D4)
	[11] = &(lt_arduino_pin_info_list[5]),  // GPIO11 (D5)
	[21] = &(lt_arduino_pin_info_list[10]), // GPIO21 (D10)
	[23] = &(lt_arduino_pin_info_list[3]),  // GPIO23 (D3)
	[24] = &(lt_arduino_pin_info_list[6]),  // GPIO24 (D6)
	[26] = &(lt_arduino_pin_info_list[7]),  // GPIO26 (D7)
};
// clang-format on
