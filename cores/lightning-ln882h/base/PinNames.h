#ifndef _PINNAMES_H_
#define _PINNAMES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/hal_gpio.h>

typedef enum {
	PORT_A = 0,
	PORT_B = 1,
} ln_PinPort;

typedef enum {
	PA_0 = 0,
	PA_1,
	PA_2,
	PA_3,
	PA_4,
	PA_5,
	PA_6,
	PA_7,
	PA_8,
	PA_9,
	PA_10,
	PA_11,
	PA_12,
	PA_13,
	PA_14,
	PA_15,

	PB_0,
	PB_1,
	PB_2,
	PB_3,
	PB_4,
	PB_5,
	PB_6,
	PB_7,
	PB_8,
	PB_9,

	// Not connected
	NC = -1
} ln_PinName;

#ifdef __cplusplus
}
#endif

#endif
