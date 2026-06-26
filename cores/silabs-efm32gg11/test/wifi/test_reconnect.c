/* Host unit tests for the STA auto-reconnect decision logic.
 * Pure C, no hardware. Run: make -C cores/silabs-efm32gg11/test/wifi */
#include "lt_wifi_reconnect.h"
#include <assert.h>

// A fully-armed, connected, healthy baseline; individual tests tweak one field.
static lt_wifi_rc_input_t base(void) {
	lt_wifi_rc_input_t in = {
		.enabled	   = true,
		.armed		   = true,
		.have_ssid	   = true,
		.status		   = LT_WIFI_RC_CONNECTED,
		.now_ms		   = 1000000,
		.last_join_ms  = 0,
		.backoff_ms	   = 0,
		.health_bad_ms = 0,
	};
	return in;
}

int main(void) {
	// --- gating: never act unless enabled + armed + have_ssid ---------------
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST; // would otherwise JOIN
		in.enabled			  = false;
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST;
		in.armed			  = false; // never reached GOT_IP: app's call, don't loop
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST;
		in.have_ssid		  = false;
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}

	// --- healthy connected link: leave it alone -----------------------------
	{
		lt_wifi_rc_input_t in = base(); // CONNECTED, health_bad_ms = 0
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.health_bad_ms	  = LT_WIFI_RC_HEALTH_TIMEOUT_MS - 1; // not yet over threshold
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}

	// --- PRIMARY FIX: silently-dead connected link demotes to retry ---------
	{
		lt_wifi_rc_input_t in = base();
		in.health_bad_ms	  = LT_WIFI_RC_HEALTH_TIMEOUT_MS; // link unhealthy long enough
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_DEMOTE);
	}

	// --- retryable states fire a join (first attempt is immediate) ----------
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST; // last_join_ms = 0 -> no backoff gate
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_JOIN);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_FAILED;
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_JOIN);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_DISCONNECTED;
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_JOIN);
	}

	// --- backoff gate holds off a too-soon retry, then releases -------------
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST;
		in.backoff_ms		  = 4000;
		in.last_join_ms		  = in.now_ms - 3999; // 3999 < 4000 -> still waiting
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_LOST;
		in.backoff_ms		  = 4000;
		in.last_join_ms		  = in.now_ms - 4000; // window elapsed -> retry
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_JOIN);
	}

	// --- SECONDARY FIX: a stuck in-flight join (IDLE) times out and retries --
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_IDLE;
		in.last_join_ms		  = in.now_ms - (LT_WIFI_RC_JOIN_TIMEOUT_MS - 1); // still in flight
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}
	{
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_IDLE;
		in.last_join_ms		  = in.now_ms - LT_WIFI_RC_JOIN_TIMEOUT_MS; // no CONNECT_IND in time
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_DEMOTE);
	}
	{
		// IDLE with no recorded join (shouldn't happen post-arm) is left alone,
		// not timed out against now-0.
		lt_wifi_rc_input_t in = base();
		in.status			  = LT_WIFI_RC_IDLE;
		in.last_join_ms		  = 0;
		assert(lt_wifi_rc_decide(&in) == LT_WIFI_RC_ACTION_NONE);
	}

	// --- backoff ramp: 0 -> 2s -> 4s -> ... -> capped at 30s ----------------
	assert(lt_wifi_rc_next_backoff(0) == LT_WIFI_RC_MIN_BACKOFF_MS);
	assert(lt_wifi_rc_next_backoff(2000) == 4000);
	assert(lt_wifi_rc_next_backoff(4000) == 8000);
	assert(lt_wifi_rc_next_backoff(16000) == LT_WIFI_RC_MAX_BACKOFF_MS); // 32000 capped
	assert(lt_wifi_rc_next_backoff(30000) == LT_WIFI_RC_MAX_BACKOFF_MS); // saturates

	return 0;
}
