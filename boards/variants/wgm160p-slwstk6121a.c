/* This file is hand-written for the WGM160P SLWSTK6121A board.
 *
 * See wgm160p-slwstk6121a.h for why boardgen 0.12.0 cannot autogenerate it.
 *
 * The 'gpio' field stores the EFM32 (port_index << 4) | pin_number encoding;
 * cores/silabs-efm32gg11/arduino/src/ArduinoFamily.h provides
 * helpers/macros to decode it into emlib GPIO_Port_TypeDef / pin pairs.
 */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PA4 — WSTK LED0 (active-high)
	{0x04u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D1: PA5 — WSTK LED1 (active-high)
	{0x05u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D2: PD6 — WSTK BTN0 (active-low)
	{0x36u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D3: PD8 — WSTK BTN1 (active-low)
	{0x38u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D4: PE6 — USART0 LOC1 RX (VCOM)
	{0x46u, PIN_GPIO | PIN_IRQ | PIN_UART,               PIN_NONE, 0},
	// D5: PE7 — USART0 LOC1 TX (VCOM)
	{0x47u, PIN_GPIO | PIN_IRQ | PIN_UART,               PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[PINS_GPIO_MAX + 1] = {
	[0x04] = &(lt_arduino_pin_info_list[0]), // PA4 (D0)
	[0x05] = &(lt_arduino_pin_info_list[1]), // PA5 (D1)
	[0x36] = &(lt_arduino_pin_info_list[2]), // PD6 (D2)
	[0x38] = &(lt_arduino_pin_info_list[3]), // PD8 (D3)
	[0x46] = &(lt_arduino_pin_info_list[4]), // PE6 (D4)
	[0x47] = &(lt_arduino_pin_info_list[5]), // PE7 (D5)
};
// clang-format on
