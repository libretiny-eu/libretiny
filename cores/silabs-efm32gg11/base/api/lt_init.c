/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 lt_init_family() for silabs-efm32gg11.
 *
 * NOTE: LibreTiny's common API (cores/common/base/api/lt_init.h) declares
 * lt_init_family(), lt_init_variant(), lt_init_arduino() as weak hooks;
 * the family implements lt_init_family(). The plan's working title "lt_init"
 * was a shorthand — the actual symbol is lt_init_family.
 */

#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_usart.h"
#include "lt_family.h"

#include <libretiny.h>

void lt_init_family(void) {
	CHIP_Init();

	// DCDC first, before any clock ramp. On the WGM160P the GG11's DCDC
	// output (1.8 V, 4.7 uH on VREGSW) feeds not only the GG11's DVDD but
	// the WF200's VDDRF/VDDDIG (module DS Figure 3.1). Out of POR the DCDC
	// is in bypass, so those rails sit at raw VBAT (3.3 V) and the WF200
	// never boots — its host interface stays mute on both SDIO and SPI
	// (bench-proven 2026-06-11 via GPIO bitbang: CMD5 dead in bypass,
	// answers on the first attempt once the DCDC regulates).
	//
	// Use the WGM160P-module parameters from em_emu.h's
	// `#elif defined(WGM160PX22KGA2)` EMU_DCDCINIT_DEFAULT (we build with
	// the plain EFM32GG11B device define, so that branch never selects
	// itself): analog peripherals on AVDD, and 4.7 uF low-noise loop
	// compensation matching the module's actual output capacitor. The
	// generic default's 1 uF compensation destabilizes the loop once the
	// WF200 loads the rail (bench: SDIO enum passed, then brownout reset
	// during radio bring-up, boot-looping).
	EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
	// CRITICAL: the generic-EFM32 EMU_DCDCINIT_DEFAULT (our device define's
	// branch) is emuDcdcMode_LowPower in EM0 — a ~10 mA regulator. The WF200
	// enumerates and accepts the firmware download on that, then collapses
	// DVDD the instant the radio firmware starts (bench: DVDDBOD boot loop;
	// Silabs demo dump showed DCDCCTRL=0x31 LOWNOISE vs our 0x32 LOWPOWER).
	dcdcInit.dcdcMode			= emuDcdcMode_LowNoise;
	dcdcInit.anaPeripheralPower = emuDcdcAnaPeripheralPower_AVDD;
	dcdcInit.dcdcLnCompCtrl		= emuDcdcLnCompCtrl_4u7F;
	// The module default's high-efficiency (DCM) reverse-current mode has
	// slow transient response; the WF200's radio load steps brown out DVDD
	// (bench: RSTCAUSE.DVDDBOD boot loop during radio bring-up). Forced-CCM
	// fast-transient mode + realistic load estimate handle the steps.
	dcdcInit.reverseCurrentControl = emuDcdcLnFastTransient;
	dcdcInit.em01LoadCurrent_mA	   = 80;
	EMU_DCDCInit(&dcdcInit);
	// KNOWN OPEN ISSUE (2026-06-11): with the DCDC regulating, the WF200
	// answers CMD5/enumerates/accepts the full firmware download, but DVDD
	// brown-out-resets the moment the radio firmware starts executing
	// (RSTCAUSE.DVDDBOD; deterministic at fw_offset == sl_wfx_firmware_size).
	// Reproduced from clean POR, at 19 MHz, with mVout 1800/1900, DCM and
	// forced-CCM. Next discriminator: Silabs reference demo on this unit.

	// Clock tree: HFXO (50 MHz) -> DPLL disciplines HFRCO to 72 MHz -> HFCLK.
	//
	// CMU_HFXOINIT_DEFAULT leaves CTUNE (crystal load capacitance) at the
	// register default of 0 — and with zero load tuning the WGM160P's 50 MHz
	// crystal never reaches HFXORDY (bench-confirmed: HFXOENS set, RDY never).
	// Mirror the GSDK's sl_device_init_hfxo_s1.c CTUNE selection: factory
	// module calibration from DEVINFO->MODXOCAL when present, else the MFG
	// token in the user-data page, else the sl_device_init config fallback
	// of 360 used for WGM160P boards (BRD4321A). The bench JuiceBox has no
	// DEVINFO cal (MODULEINFO.HFXOCALVAL=1) and an erased UD page, so it
	// takes the 360 fallback.
	CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
	int32_t ctune				  = -1;
	if ((DEVINFO->MODULEINFO & _DEVINFO_MODULEINFO_HFXOCALVAL_MASK) == 0) {
		ctune = DEVINFO->MODXOCAL & _DEVINFO_MODXOCAL_HFXOCTUNE_MASK;
	}
	const uint16_t mfgCtune = *((const uint16_t *)0x0FE00100UL); // MFG token, UD page
	if (ctune == -1 && mfgCtune != 0xFFFF) {
		ctune = mfgCtune;
	}
	if (ctune == -1) {
		ctune = 360;
	}
	hfxoInit.ctuneSteadyState = (uint16_t)ctune;
	CMU_HFXOInit(&hfxoInit);

	// Enable HFXO with a BOUNDED ready-wait instead of emlib's wait=true:
	// if the crystal ever fails to start, an unbounded wait hangs boot before
	// any observable sign of life (no LED, no UART, no fault dump). A dead
	// crystal degrades to staying on the 19 MHz HFRCO instead.
	CMU_OscillatorEnable(cmuOsc_HFXO, true, false);
	for (uint32_t t = 4000000U; t && !(CMU->STATUS & CMU_STATUS_HFXORDY); t--) {}

	if (CMU->STATUS & CMU_STATUS_HFXORDY) {
		// Run from HFXO (50 MHz, known-good) while the DPLL retunes the HFRCO.
		CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

		// DPLL configuration: 72 MHz from 50 MHz HFXO.
		// Ratio = (N+1) / (M+1).  N=71, M=49  ->  72/50 = 1.44, target 72 MHz.
		// NOTE: GG11's CMU_DPLLInit_TypeDef (em_cmu.h Series-1 variant) has no
		// .ditherEn member — spread-spectrum is controlled via ssInterval/
		// ssAmplitude (kept at zero here to disable SSC).
		CMU_DPLLInit_TypeDef dpllInit = {
			.frequency	 = 72000000U,
			.n			 = 71U,
			.m			 = 49U,
			.ssInterval	 = 0U,
			.ssAmplitude = 0U,
			.refClk		 = cmuDPLLClkSel_Hfxo,
			.edgeSel	 = cmuDPLLEdgeSel_Fall,
			.lockMode	 = cmuDPLLLockMode_Phase,
			.autoRecover = true,
		};
		if (CMU_DPLLLock(&dpllInit)) {
			// On Series 1 the DPLL output IS the HFRCO, and CMU_DPLLLock does
			// NOT touch the HFCLK mux (it only handles the case where HFCLK
			// already runs from HFRCO). Select it explicitly — without this
			// the CPU keeps running from the 50 MHz HFXO while everything
			// reports a self-consistent (but 28% slow) system.
			CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
		}
		// else: DPLL failed to lock; remain on HFXO 50 MHz (degraded but
		// functional). Visible in lt_cpu_get_freq().
	} else {
		// Stop driving the dead oscillator before falling back.
		CMU_OscillatorEnable(cmuOsc_HFXO, false, false);

		// No HFXO. Bench-confirmed on a JuiceBox 40 (WiFi variant): the GG11's
		// HFXO never reaches HFXORDY in any mode (crystal/AC/DC/digital ext)
		// with correct CTUNE and full power — the GG11-side 50 MHz source is
		// simply not usable on that PCB. This does not affect the WF200 (it
		// is on SDIO with its own clocking and ran WiFi under stock Gecko OS,
		// which also operated the GG11 without HFXO). The chip is fully
		// usable without it:
		//   1. HFRCO 72 MHz band, factory-calibrated in DEVINFO->HFRCOCAL16
		//      (DWT-measured 72.1 MHz on the bench unit; ±1-3% over temp).
		//   2. DPLL-discipline the HFRCO to the LFXO when present: 32768 Hz
		//      x 2197 = 71.991 MHz (-0.012%), crystal-accurate for UART and
		//      timekeeping. The JuiceBox's 32.768 kHz RTC crystal is present
		//      and bench-confirmed to start with default LFXO settings.
		CMU_HFRCOBandSet(cmuHFRCOFreq_72M0Hz);

		CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
		CMU_LFXOInit(&lfxoInit);
		CMU_OscillatorEnable(cmuOsc_LFXO, true, false);
		// LFXO startup is slow (hundreds of ms); bounded wait ~2 s at 72 MHz.
		for (uint32_t t = 30000000U; t && !(CMU->STATUS & CMU_STATUS_LFXORDY); t--) {}

		if (CMU->STATUS & CMU_STATUS_LFXORDY) {
			CMU_DPLLInit_TypeDef dpllInit = {
				.frequency	 = 71991296U, // 32768 * 2197
				.n			 = 2196U,
				.m			 = 0U,
				.ssInterval	 = 0U,
				.ssAmplitude = 0U,
				.refClk		 = cmuDPLLClkSel_Lfxo,
				.edgeSel	 = cmuDPLLEdgeSel_Fall,
				.lockMode	 = cmuDPLLLockMode_Phase,
				.autoRecover = true,
			};
			(void)CMU_DPLLLock(&dpllInit);
			// Lock failure leaves the factory 72 MHz band — still fine.
		}
	}

	// Enable peripheral clocks needed by Phase 1 surface.
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART0, true); // WSTK virtual COM (USART0 LOC1)
	CMU_ClockEnable(cmuClock_TIMER0, true); // micros() resolution

	// Pre-configure VCOM USART pins so printf/Serial work as soon as they're called.
	// PE7 = TX, PE6 = RX (USART0 LOC1).
	GPIO_PinModeSet(gpioPortE, 7U, gpioModePushPull, 1U); // TX idle high
	GPIO_PinModeSet(gpioPortE, 6U, gpioModeInput, 0U);
}
