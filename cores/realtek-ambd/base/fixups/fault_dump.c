/* Copyright (c) Bl00d-B0b 2026-08-07. */

// Bench diagnostics (-DLT_AMBD_FAULT_DUMP=1): installed into the RAM vector
// table, prints the stacked frame and fault status and halts, where the SDK's
// HardFault patch prints one line and resets.

#if LT_AMBD_FAULT_DUMP

#include <libretiny.h>
#include <sdk_private.h>

extern void putchar_p(char c, unsigned long port);
extern uint8_t lt_uart_port;

static void put(const char *s) {
	while (*s)
		putchar_p(*s++, lt_uart_port);
}

static void put_hex(const char *name, uint32_t value) {
	static const char digits[] = "0123456789abcdef";
	char buf[16];
	put(name);
	put(" 0x");
	for (int i = 7; i >= 0; i--)
		buf[7 - i] = digits[(value >> (i * 4)) & 0xF];
	buf[8]	= '\r';
	buf[9]	= '\n';
	buf[10] = 0;
	put(buf);
}

void lt_fault_dump_c(uint32_t *frame) {
	put("\r\n*** LT HARDFAULT ***\r\n");
	put_hex("R0  ", frame[0]);
	put_hex("R1  ", frame[1]);
	put_hex("R2  ", frame[2]);
	put_hex("R3  ", frame[3]);
	put_hex("R12 ", frame[4]);
	put_hex("LR  ", frame[5]);
	put_hex("PC  ", frame[6]);
	put_hex("xPSR", frame[7]);
	put_hex("CFSR", SCB->CFSR);
	put_hex("HFSR", SCB->HFSR);
	put_hex("MMAR", SCB->MMFAR);
	put_hex("BFAR", SCB->BFAR);
	put("*** halted ***\r\n");
	while (1) {}
}

__attribute__((naked)) static void lt_fault_handler(void) {
	__asm volatile("tst lr, #4\n"
				   "ite eq\n"
				   "mrseq r0, msp\n"
				   "mrsne r0, psp\n"
				   "b lt_fault_dump_c\n");
}

void lt_fault_dump_install(void) {
	// exception 3 = HardFault, 4 = MemManage, 5 = BusFault, 6 = UsageFault
	NewVectorTable[3] = (HAL_VECTOR_FUN)lt_fault_handler;
	NewVectorTable[4] = (HAL_VECTOR_FUN)lt_fault_handler;
	NewVectorTable[5] = (HAL_VECTOR_FUN)lt_fault_handler;
	NewVectorTable[6] = (HAL_VECTOR_FUN)lt_fault_handler;
	// enable the configurable faults so they report their own cause
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk;
}

#endif // LT_AMBD_FAULT_DUMP
