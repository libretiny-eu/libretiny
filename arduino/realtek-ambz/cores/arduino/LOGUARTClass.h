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

#pragma once

#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

// TODO this class begs to be rewritten :(

class LOGUARTClass : public HardwareSerial {
  public:
	LOGUARTClass(int dwIrq, RingBuffer *pRx_buffer);

	void begin(const uint32_t dwBaudRate);

	inline void begin(const uint32_t dwBaudRate, uint16_t config) {
		begin(dwBaudRate); // TODO implement this properly
	}

	void end(void);
	int available(void);
	int peek(void);
	int read(void);
	void flush(void);
	size_t write(const uint8_t c);

	using Print::write; // pull in write(str) and write(buf, size) from Print

	operator bool() {
		return true; // UART always active
	}

  protected:
	RingBuffer *_rx_buffer;

	int _dwIrq;

  private:
	friend bool Serial_available();
};
