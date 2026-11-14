/* Temporary LED identification helper (bring-up only).
 *
 * The factory firmware drives three GPIO-table pins as outputs; the populated
 * status indicator on this module is not confirmed. This helper runs a
 * repeating pattern with a DISTINCT flash count per pin so a human can report
 * which count was seen:
 *
 *   PC_2 -> 1 slow flash, pause
 *   PC_3 -> 2 slow flashes, pause
 *   PC_0 -> 3 slow flashes, pause
 *   ... repeats
 *
 * Driven from the diagnostic heartbeat timer (2 s per state), so each pin's
 * own slot is unique in time and count. Early idle is HIGH. gpio_init/
 * gpio_change_dir/gpio_write are register-only SDK calls that allocate nothing.
 * gpio_change_dir is required: gpio_dir re-enters HAL_GPIO_Init and is rejected
 * for an already-claimed pin, leaving the pad as an input.
 *
 * Remove with LT_AMB1_LED_IDENTIFY once the pin is known. */
/* Retired after identifying PC3. Retain the experiment as historical source,
 * but never claim or drive PC0/PC2 (or the application's PC3 status LED). */
#if 0
#include <ameba_soc.h>
#include "PinNames.h"
#include "gpio_api.h"

#define LED_ID_PIN_COUNT 3u

static const PinName s_pins[LED_ID_PIN_COUNT]	   = {PC_2, PC_3, PC_0};
static const char *const s_names[LED_ID_PIN_COUNT] = {"PC_2", "PC_3", "PC_0"};
static gpio_t s_gpio[LED_ID_PIN_COUNT];
static unsigned s_idx;	  /* current pin */
static unsigned s_state;  /* state within the current pin's slot */
static int s_started;

static void led_id_begin_pin(unsigned i) {
	gpio_init(&s_gpio[i], s_pins[i]);
	gpio_change_dir(&s_gpio[i], PIN_OUTPUT);
	gpio_write(&s_gpio[i], 1);
	DiagPrintf("[amb1] led-id %s: %u slow flash(es)\r\n", s_names[i], (unsigned)(i + 1u));
}

void lt_amb1_led_identify_step(void) {
	unsigned i = s_idx;
	if (i >= LED_ID_PIN_COUNT) {
		s_idx = 0;
		i	   = 0;
	}
	if (!s_started) {
		s_started = 1;
	}
	if (s_state == 0)
		led_id_begin_pin(i);

	unsigned flashes = i + 1u;			/* 1, 2, 3 */
	unsigned blink	  = flashes * 2u;	/* states used by on/off pairs */
	if (s_state < blink) {
		int high = (s_state & 1u) ? 1 : 0; /* LOW first, then HIGH */
		gpio_write(&s_gpio[i], high);
		s_state++;
		return;
	}
	/* pause state: hold HIGH, then advance to the next pin */
	gpio_write(&s_gpio[i], 1);
	s_state = 0;
	s_idx	= (i + 1u) % LED_ID_PIN_COUNT;
}
#endif

/* Compatibility for older diagnostic call sites; deliberately no GPIO work. */
void lt_amb1_led_identify_step(void) {}
