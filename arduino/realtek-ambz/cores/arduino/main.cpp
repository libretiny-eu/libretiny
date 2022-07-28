/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>

extern "C" {

#include <cmsis_os.h>

extern uint32_t GlobalDebugEnable;
extern uint16_t GlobalDebugLevel;
extern uint8_t GlobalPrivateLog;
extern uint8_t lt_uart_port;

} // extern "C"

osThreadId main_tid = 0;

void initArduino() {
	// make the SDK less verbose by default
	GlobalDebugEnable = 0;
	GlobalPrivateLog  = 0;
	lt_uart_port	  = LT_UART_DEFAULT_PORT;
}

bool startMainTask() {
	osThreadDef(mainTask, osPriorityRealtime, 1, 4096 * 4);
	main_tid = osThreadCreate(osThread(mainTask), NULL);
	osKernelStart();
	return true;
}

void wait_for_debug() {
	while (((CoreDebug->DHCSR) & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		asm("nop");
	}
	delay(1000);
}
