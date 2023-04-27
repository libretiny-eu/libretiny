# GPIO usage instructions

!!! note
	This has changed since v1.0.0. Refer to the [migration guide](../dev/migration_v1.0.0.md) for more info.

GPIO pins can be accessed in a few ways:

- using the raw GPIO number of the chip
- using the "function macro" of the pin
- using Arduino digital pin numbers (`D#`, deprecated)

This applies both to Arduino code and ESPHome YAML configs.

## GPIO numbers - Arduino only

The simplest form of GPIO access is by using raw pin numbers of the CPU (just like on ESP8266/ESP32). For example, to access GPIO6, write `digitalRead(6)`.

## Function macros - Arduino & ESPHome

If you go to a board documentation page (like [this one - CB2S](../../boards/cb2s/README.md)) you'll see a pinout drawing, containing various labels in small blocks. There's also a table below to make the pinout a bit more clear.

**You can use any of these labels** to access a particular pin. For example, to access GPIO6, which is also the PWM0 pin on CB2S, one can use:

- `digitalRead(PIN_P6)` (Arduino) or `pin: P6` (ESPHome)
- `digitalRead(PIN_PWM0)` (Arduino) or `pin: PWM0` (ESPHome)

## Arduino `D#` numbers (deprecated)

This method of accessing pins is deprecated since v1.0.0, and **will** (probably) **be removed** in the future. It's kept for legacy compatibility.

To access the previously mentioned GPIO6, use `digitalRead(D2)` or `digitalRead(PIN_D2)` or `pin: D2`. Note that the `D#` pin numbers are not consistent between boards (for example, on WB3S, GPIO6 is already D4).
