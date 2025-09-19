/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <sdk_private.h>

extern "C" {

osThreadId main_tid = 0;

#if LT_AUTO_DOWNLOAD_REBOOT && LT_ARD_HAS_SERIAL && LT_HW_UART2
void lt_init_arduino() {
	// initialize auto-download-reboot parser
	Serial2.begin(115200);
}
#endif

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

} // extern "C"
