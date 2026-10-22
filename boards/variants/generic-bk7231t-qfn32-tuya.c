/* This file was auto-generated from generic-bk7231t-qfn32-tuya.json using boardgen */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: P0, UART2_TX, I2C2_SCL
	{GPIO0,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D1: P1, UART2_RX, I2C2_SDA
	{GPIO1,  PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_UART,           PIN_NONE, 0},
	// D2: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D3: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D4: P8, PWM2
	{GPIO8,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D5: P9, PWM3
	{GPIO9,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D6: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D7: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D8: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D9: P15, SD_CMD, CS
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D10: P16, SD_D0, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D11: P17, SD_D1, MISO
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D12: P20, I2C1_SCL, TCK, FSCK
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG,           PIN_NONE, 0},
	// D13: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
	// D14: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,                     PIN_NONE, 0},
	// D15: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D16: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D17: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D18: P28, DN
	{GPIO28, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[0]),  // GPIO0 (D0)
	[1]  = &(lt_arduino_pin_info_list[1]),  // GPIO1 (D1)
	[6]  = &(lt_arduino_pin_info_list[2]),  // GPIO6 (D2)
	[7]  = &(lt_arduino_pin_info_list[3]),  // GPIO7 (D3)
	[8]  = &(lt_arduino_pin_info_list[4]),  // GPIO8 (D4)
	[9]  = &(lt_arduino_pin_info_list[5]),  // GPIO9 (D5)
	[10] = &(lt_arduino_pin_info_list[6]),  // GPIO10 (D6)
	[11] = &(lt_arduino_pin_info_list[7]),  // GPIO11 (D7)
	[14] = &(lt_arduino_pin_info_list[8]),  // GPIO14 (D8)
	[15] = &(lt_arduino_pin_info_list[9]),  // GPIO15 (D9)
	[16] = &(lt_arduino_pin_info_list[10]), // GPIO16 (D10)
	[17] = &(lt_arduino_pin_info_list[11]), // GPIO17 (D11)
	[20] = &(lt_arduino_pin_info_list[12]), // GPIO20 (D12)
	[21] = &(lt_arduino_pin_info_list[13]), // GPIO21 (D13)
	[22] = &(lt_arduino_pin_info_list[14]), // GPIO22 (D14)
	[23] = &(lt_arduino_pin_info_list[15]), // GPIO23 (D15)
	[24] = &(lt_arduino_pin_info_list[16]), // GPIO24 (D16)
	[26] = &(lt_arduino_pin_info_list[17]), // GPIO26 (D17)
	[28] = &(lt_arduino_pin_info_list[18]), // GPIO28 (D18)
};
// clang-format on
