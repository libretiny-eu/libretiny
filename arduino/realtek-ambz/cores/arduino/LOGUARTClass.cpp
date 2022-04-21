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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LOGUARTClass.h"

#define LOG_UART_MODIFIABLE_BAUD_RATE 1

#ifdef __cplusplus
extern "C" {
#endif

#include "log_uart_api.h"
#include "hal_irqn.h"

log_uart_t log_uart_obj;

#ifdef __cplusplus
}
#endif

RingBuffer rx_buffer0;

void arduino_loguart_irq_handler(uint32_t id, LOG_UART_INT_ID event)
{
    char c;
    RingBuffer *pRxBuffer = (RingBuffer *)id;

    if (event == IIR_RX_RDY || IIR_CHAR_TIMEOUT) {
        c = log_uart_getc(&log_uart_obj);
        pRxBuffer->store_char(c);
    }
}

LOGUARTClass::LOGUARTClass(int dwIrq, RingBuffer* pRx_buffer )
{
    _rx_buffer = pRx_buffer;
    _dwIrq = dwIrq;
}

// Protected Methods //////////////////////////////////////////////////////////////




// Public Methods //////////////////////////////////////////////////////////////


void LOGUARTClass::IrqHandler( void )
{

    uint8_t     data = 0;
    BOOL    	PullMode = _FALSE;
    uint32_t 	IrqEn = DiagGetIsrEnReg();

    DiagSetIsrEnReg(0);

    data = DiagGetChar(PullMode);
	if ( data > 0 ) 
		_rx_buffer->store_char(data);

    DiagSetIsrEnReg(IrqEn);

}


void LOGUARTClass::begin( const uint32_t dwBaudRate )
{
#if LOG_UART_MODIFIABLE_BAUD_RATE
    /* log uart initialize in 38400 baud rate.
     * If we change baud rate here, Serail Monitor would not detect this change and show nothing on screen.
     */
    log_uart_init(&log_uart_obj, dwBaudRate, 8, ParityNone, 1);
#else
    log_uart_init(&log_uart_obj, 38400, 8, ParityNone, 1);
#endif
    log_uart_irq_set(&log_uart_obj, IIR_RX_RDY, 1);
    log_uart_irq_handler(&log_uart_obj, arduino_loguart_irq_handler, (uint32_t)_rx_buffer);
}

void LOGUARTClass::end( void )
{
    // clear any received data
    _rx_buffer->_iHead = _rx_buffer->_iTail ;

    log_uart_free(&log_uart_obj);
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
    log_uart_putc(&log_uart_obj, uc_data);
  	return 1;
}

LOGUARTClass Serial(UART_LOG_IRQ, &rx_buffer0);

bool Serial_available() {
    return Serial.available() > 0;
}
