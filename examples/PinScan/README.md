# PinScan

This example allows to quickly check all digital/analog pins of an IoT device.

By using a simple TUI (text user interface), you can check which I/O pins correspond to i.e. button presses, as well as write to one of the pins to see which LED lights up.

!!! warning
	Messing with pins in a device can potentially damage some parts of it. Be careful when writing voltages to digital and PWM pins.

Uploading the example and opening up a terminal (e.g. PuTTY) presents this menu:
```
LibreTuya v0.8.0, PinScan v1.0
Board: cb2s
I/O count: 11
Digital I/O count: 11
Analog input count: 1
-------- UART2 --------
Commands:
        d - Check digital pins
        a - Check analog pins
        s - Select UART port
        t - Toggle ANSI control codes
        r - Reboot (for uploading)
        q - Go back to menu, at any time
        ? - Print help text, also for subcommands
```

The interface expects one-letter commands, without confirmation by `Enter`. The only part which expects an `Enter` keypress is inputting pin numbers and UART port numbers.

Pressing `q` at any time goes back to the main menu, terminating the current process.

!!! note
	PinScan works best with a terminal supporting ANSI escape codes (PuTTY does), but this behavior can be disabled using `t`.

Switching to another UART port is possible (for example if the default port is on the pins you want to check) using `s` command. Do not change the port after using any I/O commands, as it won't work; reboot it using `r` before.

By setting `USE_WIFI` in `main.h` to 1, a Telnet server is enabled on port 23. This allows to test I/O pins without having physical, wired access to the device (i.e. using OTA). Make sure to specify correct WiFi credentials.

!!! hint
	If your board isn't supported by LT yet, use one of the generic boards.

	If your board doesn't even have a known pinout, use `d`/`s` commands of PinScan to ease the mapping of all board pins.

## Digital pins

```
Digital I/O
-------- UART2 --------
Commands:
        r - Realtime readout of all pins
        o - Read one pin continuously
        s - Manual Scan - toggle each pin
        h - Write HIGH to a pin
        l - Write LOW to a pin
        p - Output using pull up/down (default)
        w - Output using write low/high (less safe)
```

### Realtime readout of all pins

```
 D0  D1  D2  D3  D4  D5  D6  D7  D8  D9 D10
 LO  LO  HI  HI  HI  HI  LO  LO  --  --  LO
```

Screen contents will update when voltage on one of the pins changes. Pins marked with `--` mean the currently used UART port (which can be changed using `s` command; after reboot).

!!! tldr
	Try pressing a button to see which pin changes.

### Read one pin continuously

Enter the pin number, it will be probed until you press `q`.

### Manual Scan - toggle each pin

```
 D0  D1  D2  D3  D4  D5  D6  D7  D8  D9 D10
 HI  LO  LO  LO  LO  LO  LO  LO  --  --  LO
```

A pin will be toggled every 500ms, starting with D0. Type `n` to move to the next pin.

!!! tldr
	Go through the pins to see which lights up an LED.

### Write HIGH/LOW to a pin

Self-explanatory.

### Output using pull/write

Outputs can be toggled by using internal pull-up/pull-down resistors, or by simply writing a voltage. Writing is more current-efficient, but is also less safe if something else supplies different voltage to the pin.

This affects scan and write high/low commands.

!!! tldr
	Use `write` output mode (carefully) if there's an LED which doesn't light up with default pull mode.

## Analog pins

```
Analog inputs
-------- UART2 --------
Commands:
        r - Realtime readout of all pins
        o - Read one pin once
```

### Realtime readout of all pins

Read voltage (in millivolts) on all available analog pins, until `q` is pressed.

### Read one pin once

No need to explain.
