/* This file was auto-generated from generic-bk7252.json using boardgen */

#include <Arduino.h>

extern "C" {

// clang-format off
PinInfo pinTable[PINS_COUNT] = {
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
	// D17: P19, SD_D1
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
// clang-format on

} // extern "C"
