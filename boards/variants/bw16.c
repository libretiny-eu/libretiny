/* BW16 (RTL8720DN) — pin roles from the official Realtek Arduino variant
 * (ameba-arduino-d, rtl8720dn_bw16). */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA7, LOG_TX
	{7u,  PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D1: PA8, LOG_RX
	{8u,  PIN_GPIO | PIN_IRQ | PIN_UART,           PIN_NONE, 0},
	// D2: PA27, SWD_DATA
	{27u, PIN_GPIO | PIN_IRQ,                      PIN_NONE, 0},
	// D3: PA30, PWM
	{30u, PIN_GPIO | PIN_IRQ | PIN_PWM,            PIN_NONE, 0},
	// D4: PB1, A0, SERIAL1_TX
	{33u, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART, PIN_NONE, 0},
	// D5: PB2, A1, SERIAL1_RX
	{34u, PIN_GPIO | PIN_IRQ | PIN_ADC | PIN_UART, PIN_NONE, 0},
	// D6: PB3, A2, SWD_CLK
	{35u, PIN_GPIO | PIN_IRQ | PIN_ADC,            PIN_NONE, 0},
	// D7: PA25, PWM, I2C_SCL
	{25u, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C,  PIN_NONE, 0},
	// D8: PA26, PWM, I2C_SDA
	{26u, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_I2C,  PIN_NONE, 0},
	// D9: PA15, SPI_SS
	{15u, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
	// D10: PA14, SPI_SCLK, LED_G — no PWM: the TypeC Arduino docs claim it,
	// but the SDK pwmout pinmap has no PA14 entry and the driver hangs (bench)
	{14u, PIN_GPIO | PIN_IRQ | PIN_SPI,            PIN_NONE, 0},
	// D11: PA13, PWM, SPI_MISO, LED_B
	{13u, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI,  PIN_NONE, 0},
	// D12: PA12, PWM, SPI_MOSI, LED_R
	{12u, PIN_GPIO | PIN_IRQ | PIN_PWM | PIN_SPI,  PIN_NONE, 0},
};
// clang-format on

// Maps a GPIO number to its PinInfo; indices not listed stay NULL.
PinInfo *lt_arduino_pin_gpio_map[PINS_GPIO_MAX + 1] = {
	[7]	 = &(lt_arduino_pin_info_list[0]),	// D0
	[8]	 = &(lt_arduino_pin_info_list[1]),	// D1
	[27] = &(lt_arduino_pin_info_list[2]),	// D2
	[30] = &(lt_arduino_pin_info_list[3]),	// D3
	[33] = &(lt_arduino_pin_info_list[4]),	// D4
	[34] = &(lt_arduino_pin_info_list[5]),	// D5
	[35] = &(lt_arduino_pin_info_list[6]),	// D6
	[25] = &(lt_arduino_pin_info_list[7]),	// D7
	[26] = &(lt_arduino_pin_info_list[8]),	// D8
	[15] = &(lt_arduino_pin_info_list[9]),	// D9
	[14] = &(lt_arduino_pin_info_list[10]), // D10
	[13] = &(lt_arduino_pin_info_list[11]), // D11
	[12] = &(lt_arduino_pin_info_list[12]), // D12
};

void lt_init_variant() {
	// no board-specific init on BW16 (no PSRAM, no external peripherals)
}
