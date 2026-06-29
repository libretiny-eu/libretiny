/* This file was auto-generated from generic-bk7252.json using boardgen */

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
	// D2: P2, ADC4, SCK
	{GPIO2,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2S,            PIN_NONE, 0},
	// D3: P3, ADC5, WS
	{GPIO3,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2S,            PIN_NONE, 0},
	// D4: P4, ADC1, DIN
	{GPIO4,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2S,            PIN_NONE, 0},
	// D5: P5, ADC2, DOUT
	{GPIO5,  PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_I2S,            PIN_NONE, 0},
	// D6: P6, PWM0
	{GPIO6,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D7: P7, PWM1
	{GPIO7,  PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D8: P10, UART1_RX
	{GPIO10, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D9: P11, UART1_TX
	{GPIO11, PIN_GPIO | PIN_IRQ | PIN_UART,                     PIN_NONE, 0},
	// D10: P12, ADC6, UART1_CTS
	{GPIO12, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,           PIN_NONE, 0},
	// D11: P13, ADC7, UART1_RTS
	{GPIO13, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART,           PIN_NONE, 0},
	// D12: P14, SD_CLK, SCK
	{GPIO14, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D13: P15, SD_CMD, CS
	{GPIO15, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D14: P16, SD_D0, MOSI
	{GPIO16, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D15: P17, SD_D1, MISO
	{GPIO17, PIN_GPIO | PIN_IRQ | PIN_SPI,                      PIN_NONE, 0},
	// D16: P18, SD_D2
	{GPIO18, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D17: P19, SD_D3
	{GPIO19, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D18: P20, I2C1_SCL, TCK, FSCK
	{GPIO20, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_JTAG,           PIN_NONE, 0},
	// D19: P21, I2C1_SDA, TMS, MCLK, ^FCS
	{GPIO21, PIN_GPIO | PIN_IRQ | PIN_I2C | PIN_I2S | PIN_JTAG, PIN_NONE, 0},
	// D20: P22, TDI, FSI
	{GPIO22, PIN_GPIO | PIN_IRQ | PIN_JTAG,                     PIN_NONE, 0},
	// D21: P23, ADC3, TDO, FSO
	{GPIO23, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_JTAG,           PIN_NONE, 0},
	// D22: P24, PWM4
	{GPIO24, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D23: P25, DP
	{GPIO25, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D24: P26, PWM5, IRDA
	{GPIO26, PIN_GPIO | PIN_IRQ | PIN_PWM,                      PIN_NONE, 0},
	// D25: P27, DVP_MCLK
	{GPIO27, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D26: P28, DN
	{GPIO28, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D27: P29, DVP_PCLK
	{GPIO29, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D28: P30, DVP_HSYNC
	{GPIO30, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D29: P31, DVP_VSYNC
	{GPIO31, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D30: P32, DVP_PD0
	{GPIO32, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D31: P33, DVP_PD1
	{GPIO33, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D32: P34, DVP_PD2
	{GPIO34, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D33: P35, DVP_PD3
	{GPIO35, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D34: P36, DVP_PD4
	{GPIO36, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D35: P37, DVP_PD5
	{GPIO37, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D36: P38, DVP_PD6
	{GPIO38, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
	// D37: P39, DVP_PD7
	{GPIO39, PIN_GPIO | PIN_IRQ,                                PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[] = {
	[0]  = &(lt_arduino_pin_info_list[0]),  // GPIO0 (D0)
	[1]  = &(lt_arduino_pin_info_list[1]),  // GPIO1 (D1)
	[2]  = &(lt_arduino_pin_info_list[2]),  // GPIO2 (D2)
	[3]  = &(lt_arduino_pin_info_list[3]),  // GPIO3 (D3)
	[4]  = &(lt_arduino_pin_info_list[4]),  // GPIO4 (D4)
	[5]  = &(lt_arduino_pin_info_list[5]),  // GPIO5 (D5)
	[6]  = &(lt_arduino_pin_info_list[6]),  // GPIO6 (D6)
	[7]  = &(lt_arduino_pin_info_list[7]),  // GPIO7 (D7)
	[10] = &(lt_arduino_pin_info_list[8]),  // GPIO10 (D8)
	[11] = &(lt_arduino_pin_info_list[9]),  // GPIO11 (D9)
	[12] = &(lt_arduino_pin_info_list[10]), // GPIO12 (D10)
	[13] = &(lt_arduino_pin_info_list[11]), // GPIO13 (D11)
	[14] = &(lt_arduino_pin_info_list[12]), // GPIO14 (D12)
	[15] = &(lt_arduino_pin_info_list[13]), // GPIO15 (D13)
	[16] = &(lt_arduino_pin_info_list[14]), // GPIO16 (D14)
	[17] = &(lt_arduino_pin_info_list[15]), // GPIO17 (D15)
	[18] = &(lt_arduino_pin_info_list[16]), // GPIO18 (D16)
	[19] = &(lt_arduino_pin_info_list[17]), // GPIO19 (D17)
	[20] = &(lt_arduino_pin_info_list[18]), // GPIO20 (D18)
	[21] = &(lt_arduino_pin_info_list[19]), // GPIO21 (D19)
	[22] = &(lt_arduino_pin_info_list[20]), // GPIO22 (D20)
	[23] = &(lt_arduino_pin_info_list[21]), // GPIO23 (D21)
	[24] = &(lt_arduino_pin_info_list[22]), // GPIO24 (D22)
	[25] = &(lt_arduino_pin_info_list[23]), // GPIO25 (D23)
	[26] = &(lt_arduino_pin_info_list[24]), // GPIO26 (D24)
	[27] = &(lt_arduino_pin_info_list[25]), // GPIO27 (D25)
	[28] = &(lt_arduino_pin_info_list[26]), // GPIO28 (D26)
	[29] = &(lt_arduino_pin_info_list[27]), // GPIO29 (D27)
	[30] = &(lt_arduino_pin_info_list[28]), // GPIO30 (D28)
	[31] = &(lt_arduino_pin_info_list[29]), // GPIO31 (D29)
	[32] = &(lt_arduino_pin_info_list[30]), // GPIO32 (D30)
	[33] = &(lt_arduino_pin_info_list[31]), // GPIO33 (D31)
	[34] = &(lt_arduino_pin_info_list[32]), // GPIO34 (D32)
	[35] = &(lt_arduino_pin_info_list[33]), // GPIO35 (D33)
	[36] = &(lt_arduino_pin_info_list[34]), // GPIO36 (D34)
	[37] = &(lt_arduino_pin_info_list[35]), // GPIO37 (D35)
	[38] = &(lt_arduino_pin_info_list[36]), // GPIO38 (D36)
	[39] = &(lt_arduino_pin_info_list[37]), // GPIO39 (D37)
};
// clang-format on
