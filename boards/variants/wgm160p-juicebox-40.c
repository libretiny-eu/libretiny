/* This file is hand-written for the Enel X JuiceBox 40 (WGM160P host).
 *
 * See wgm160p-juicebox-40.h for the pin-role source (bench-confirmed Gecko OS
 * GPIO map) and why boardgen 0.12.0 cannot autogenerate it.
 *
 * The 'gpio' field stores the EFM32 (port_index << 4) | pin_number encoding;
 * cores/silabs-efm32gg11/arduino/src/ArduinoFamily.h decodes it into emlib
 * GPIO_Port_TypeDef / pin pairs.
 */

#include <Arduino.h>

#ifdef LT_VARIANT_INCLUDE
#include LT_VARIANT_INCLUDE
#endif

// clang-format off
PinInfo lt_arduino_pin_info_list[PINS_COUNT] = {
	// D0: PB5 — RGB LED red, active-high (bench-verified 2026-06-11)
	{0x15u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D1: PB6 — RGB LED green, active-high (bench-verified 2026-06-11)
	{0x16u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D2: PD8 — RGB LED blue (found by SWD GPIO sweep; not in the Gecko OS map)
	{0x38u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D3: PC4 — factory_reset line (Gecko GPIO 16)
	{0x24u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
	// D4: PE6 — USART0 LOC1 RX (console)
	{0x46u, PIN_GPIO | PIN_IRQ | PIN_UART,               PIN_NONE, 0},
	// D5: PE7 — USART0 LOC1 TX (console)
	{0x47u, PIN_GPIO | PIN_IRQ | PIN_UART,               PIN_NONE, 0},
	// D6: PB3 — no visible function (Gecko OS table said RGB; drives nothing — bench-verified 2026-06-11)
	{0x13u, PIN_GPIO | PIN_IRQ,                          PIN_NONE, 0},
};

PinInfo *lt_arduino_pin_gpio_map[PINS_GPIO_MAX + 1] = {
	[0x15] = &(lt_arduino_pin_info_list[0]), // PB5 (D0)
	[0x16] = &(lt_arduino_pin_info_list[1]), // PB6 (D1)
	[0x38] = &(lt_arduino_pin_info_list[2]), // PD8 (D2)
	[0x24] = &(lt_arduino_pin_info_list[3]), // PC4 (D3)
	[0x46] = &(lt_arduino_pin_info_list[4]), // PE6 (D4)
	[0x47] = &(lt_arduino_pin_info_list[5]), // PE7 (D5)
	[0x13] = &(lt_arduino_pin_info_list[6]), // PB3 (D6)
};
// clang-format on
