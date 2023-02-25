/* Copyright (c) Kuba Szczodrzyński 2022-04-22. */

#include <ameba_soc.h>
#include <osdep_service.h>

// usage:
// extern int LOGUART_SetBaud(uint32_t BaudRate);

int LOGUART_SetBaud(uint32_t BaudRate)
{
	UART_INTConfig(UART2_DEV, RUART_IER_ERBI | RUART_IER_ELSI, DISABLE);
	UART_RxCmd(UART2_DEV, DISABLE);

	UART_SetBaud(UART2_DEV, BaudRate);

	UART_INTConfig(UART2_DEV, RUART_IER_ERBI | RUART_IER_ELSI, ENABLE);
	UART_RxCmd(UART2_DEV, ENABLE);

	return 1;
}

void LOGUART_SetBaud_FromFlash(void)
{
	// useless, nop
}

void ReRegisterPlatformLogUart(void)
{
	// useless, nop
}
