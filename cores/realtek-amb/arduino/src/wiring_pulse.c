/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <PinNames.h>
#include <gpio_api.h>
#include <objects.h>
#include <us_ticker_api.h>

extern void *gpio_pin_struct[];

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.  Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse. */
extern unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
	// cache the port and bit of the pin in order to speed up the
	// pulse width measuring loop and achieve finer resolution.  calling
	// digitalRead() instead yields much coarser resolution.
	gpio_t *pGpio_t;

	uint32_t start_ticks, cur_ticks;

	if (pin < 0 || pin > PINS_COUNT || (pinTable[pin].gpio == NC))
		return 0;

	/* Handle */
	if (pinTable[pin].enabled != PIN_GPIO) {
		return 0;
	}

	pGpio_t = (gpio_t *)gpio_pin_struct[pin];

	// wait for any previous pulse to end
	start_ticks = us_ticker_read();
	while (gpio_read(pGpio_t) == state) {
		cur_ticks = us_ticker_read();
		if (cur_ticks - start_ticks > timeout)
			return 0;
	}

	// wait for the pulse to start
	while (gpio_read(pGpio_t) != state) {
		cur_ticks = us_ticker_read();
		if (cur_ticks - start_ticks > timeout)
			return 0;
	}

	// wait for the pulse to stop
	start_ticks = us_ticker_read();
	while (gpio_read(pGpio_t) == state) {
		cur_ticks = us_ticker_read();
		if (cur_ticks - start_ticks > timeout)
			return 0;
	}

	cur_ticks = us_ticker_read();

	// convert the reading to microseconds. The loop has been determined
	// to be 52 clock cycles long and have about 16 clocks between the edge
	// and the start of the loop. There will be some error introduced by
	// the interrupt handlers.
	return cur_ticks - start_ticks;
}
