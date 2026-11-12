/* The Ameba1 log UART is separate from the mbed UART peripherals. */
#include <ameba_soc.h>
#include <hal_log_uart.h>

int LOGUART_SetBaud(uint32_t baudrate) {
	if (!baudrate)
		return 0;
	HAL_LOG_UART_ADAPTER adapter = {0};
	adapter.BaudRate = baudrate;
	HalLogUartSetBaudRate(&adapter);
	return 1;
}

void LOGUART_SetBaud_FromFlash(void) {}
void ReRegisterPlatformLogUart(void) {}
