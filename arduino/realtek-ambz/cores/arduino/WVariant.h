#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_extern.h"
#include "sdk_mem.h"
#include "sdk_os.h"

#define NOT_INITIAL	 (1UL << 0)
#define PIO_GPIO	 (1UL << 1)
#define PIO_PWM		 (1UL << 2)
#define PIO_I2C		 (1UL << 3)
#define PIO_ADC		 (1UL << 4)
#define PIO_DAC		 (1UL << 5)
#define PIO_GPIO_IRQ (1UL << 6)

#define PWM_MODE_ENABLED  1
#define PWM_MODE_DISABLED 0

#define DEFAULT	 1
#define EXTERNAL 0

#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))

typedef enum _eAnalogReference {
	AR_DEFAULT,
} eAnalogReference;

// Types used for the table below
typedef struct _PinDescription {
	// HW PinNames
	uint32_t pinname;
	// Current Pin Type
	uint32_t ulPinType;
	// Supported Pin Function
	uint32_t ulPinAttribute;
	// Current Pin Mode
	uint32_t ulPinMode;
} PinDescription;

// Pins table to be instantiated into variant.cpp
extern PinDescription g_APinDescription[];

// Additional Wiring functions
extern uint32_t digitalPinToPort(uint32_t pinNumber);
extern uint32_t digitalPinToBitMask(uint32_t pinNumber);
extern void analogOutputInit(void);
extern void wait_for_debug();

#ifdef __cplusplus
} // extern "C"
#endif
