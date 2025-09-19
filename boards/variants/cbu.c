/* This file was auto-generated from cbu.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D1: P16, SD_D0, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D2: P20, I2C1_SCL, TCK, FSCK
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG,           PIN_NONE, 0},
	// D3: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,                     PIN_NONE, 0},
	// D4: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D5: P1, UART2_RX, I2C2_SDA
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D6: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D7: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D8: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D9: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D10: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D11: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D12: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D13: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D14: P28, DN
	{GPIO28, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D15: P9, PWM3
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D16: P17, SD_D1, MISO
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D17: P15, SD_CMD, CS
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D18: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[6]),  // GPIO0 (D6)
	[1]  = &(lt_arduino_pin_info_list[5]),  // GPIO1 (D5)
	[6]  = &(lt_arduino_pin_info_list[9]),  // GPIO6 (D9)
	[7]  = &(lt_arduino_pin_info_list[8]),  // GPIO7 (D8)
	[8]  = &(lt_arduino_pin_info_list[7]),  // GPIO8 (D7)
	[9]  = &(lt_arduino_pin_info_list[15]), // GPIO9 (D15)
	[10] = &(lt_arduino_pin_info_list[13]), // GPIO10 (D13)
	[11] = &(lt_arduino_pin_info_list[12]), // GPIO11 (D12)
	[14] = &(lt_arduino_pin_info_list[0]),  // GPIO14 (D0)
	[15] = &(lt_arduino_pin_info_list[17]), // GPIO15 (D17)
	[16] = &(lt_arduino_pin_info_list[1]),  // GPIO16 (D1)
	[17] = &(lt_arduino_pin_info_list[16]), // GPIO17 (D16)
	[20] = &(lt_arduino_pin_info_list[2]),  // GPIO20 (D2)
	[21] = &(lt_arduino_pin_info_list[18]), // GPIO21 (D18)
	[22] = &(lt_arduino_pin_info_list[3]),  // GPIO22 (D3)
	[23] = &(lt_arduino_pin_info_list[4]),  // GPIO23 (D4)
	[24] = &(lt_arduino_pin_info_list[11]), // GPIO24 (D11)
	[26] = &(lt_arduino_pin_info_list[10]), // GPIO26 (D10)
	[28] = &(lt_arduino_pin_info_list[14]), // GPIO28 (D14)
};
// clang-format on
