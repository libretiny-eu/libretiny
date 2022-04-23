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

#include "Arduino.h"

#include "LOGUARTClass.h"

#define LOG_UART_MODIFIABLE_BAUD_RATE 1

#ifdef __cplusplus
extern "C" {
#endif

#include "osdep_service.h"
#include "rtl8710b.h"
extern int LOGUART_SetBaud(uint32_t BaudRate); // from fixups/log_uart.c

#ifdef __cplusplus
}
#endif

RingBuffer rx_buffer0;

LOGUARTClass::LOGUARTClass(int dwIrq, RingBuffer* pRx_buffer )
{
    _rx_buffer = pRx_buffer;
    _dwIrq = dwIrq;
}

// Protected Methods //////////////////////////////////////////////////////////////




// Public Methods //////////////////////////////////////////////////////////////


void IrqHandler( void )
{

    uint8_t     data = 0;
    BOOL    	PullMode = _FALSE;
    uint32_t 	IrqEn = DiagGetIsrEnReg();

    DiagSetIsrEnReg(0);

    data = DiagGetChar(PullMode);
	if ( data > 0 )
		rx_buffer0.store_char(data);

    DiagSetIsrEnReg(IrqEn);

}


void LOGUARTClass::begin( const uint32_t dwBaudRate )
{
    DIAG_UartReInit((IRQ_FUN) IrqHandler);
    NVIC_SetPriority(UART_LOG_IRQ, 10);
    LOGUART_SetBaud(dwBaudRate);
}

void LOGUARTClass::end( void )
{
    // clear any received data
    _rx_buffer->_iHead = _rx_buffer->_iTail ;
}

int LOGUARTClass::available( void )
{
  return (uint32_t)(SERIAL_BUFFER_SIZE + _rx_buffer->_iHead - _rx_buffer->_iTail) % SERIAL_BUFFER_SIZE ;
}

int LOGUARTClass::peek( void )
{

  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  return _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;

}

int LOGUARTClass::read( void )
{
  // if the head isn't ahead of the tail, we don't have any characters
  if ( _rx_buffer->_iHead == _rx_buffer->_iTail )
    return -1 ;

  uint8_t uc = _rx_buffer->_aucBuffer[_rx_buffer->_iTail] ;
  _rx_buffer->_iTail = (unsigned int)(_rx_buffer->_iTail + 1) % SERIAL_BUFFER_SIZE ;
  return uc ;

}

void LOGUARTClass::flush( void )
{
// TODO:
// while ( serial_writable(&(this->sobj)) != 1 );
/*
  // Wait for transmission to complete
  while ((_pUart->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY)
    ;
*/
}

size_t LOGUARTClass::write( const uint8_t uc_data )
{
    DiagPutChar(uc_data);
  	return 1;
}

LOGUARTClass Serial(UART_LOG_IRQ, &rx_buffer0);

bool Serial_available() {
    return Serial.available() > 0;
}
