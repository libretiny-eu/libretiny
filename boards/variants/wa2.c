/* This file was auto-generated from wa2.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D1: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D2: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D3: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG, PIN_NONE, 0},
	// D4: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D5: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D6: P18, SD_D2, PWM4
	{GPIO18, PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D7: P19, SD_D3, PWM5
	{GPIO19, PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D8: P20, I2C1_SCL, TCK, FSCK
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG, PIN_NONE, 0},
	// D9: P4, ADC1, DIN
	{GPIO4,  PIN_GPIO | PIN_IRQ | PIN_I2S,            PIN_NONE, 0},
	// D10: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART, PIN_NONE, 0},
	// D11: P21, I2C1_SDA, TMS, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG, PIN_NONE, 0},
	// D12: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,           PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[10]), // GPIO0 (D10)
	[4]  = &(lt_arduino_pin_info_list[9]),  // GPIO4 (D9)
	[6]  = &(lt_arduino_pin_info_list[2]),  // GPIO6 (D2)
	[7]  = &(lt_arduino_pin_info_list[1]),  // GPIO7 (D1)
	[8]  = &(lt_arduino_pin_info_list[0]),  // GPIO8 (D0)
	[10] = &(lt_arduino_pin_info_list[4]),  // GPIO10 (D4)
	[11] = &(lt_arduino_pin_info_list[5]),  // GPIO11 (D5)
	[18] = &(lt_arduino_pin_info_list[6]),  // GPIO18 (D6)
	[19] = &(lt_arduino_pin_info_list[7]),  // GPIO19 (D7)
	[20] = &(lt_arduino_pin_info_list[8]),  // GPIO20 (D8)
	[21] = &(lt_arduino_pin_info_list[11]), // GPIO21 (D11)
	[22] = &(lt_arduino_pin_info_list[12]), // GPIO22 (D12)
	[23] = &(lt_arduino_pin_info_list[3]),  // GPIO23 (D3)
};
// clang-format on
