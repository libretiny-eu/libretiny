
#include "bitband_io.h"

volatile uint8_t * BitBandAddr(void *addr, uint8_t bit) {
	return (volatile uint8_t *)(BITBAND_ADDR((u32)addr, bit));
}

volatile uint8_t * BitBandPeriAddr(void *addr, uint8_t bit) {
	return (volatile uint8_t *)(BITBAND_PERI((u32)addr, bit));
}

volatile uint8_t * GetOutPinBitBandAddr(PinName pin) {
	uint32_t paddr = NULL;
	uint32_t ippin = HAL_GPIO_GetIPPinName_8195a(pin);
	if(ippin < 0xff) {
		// paddr = 0x42000000 + (0x40001000 + 0x0c * (ippin >> 5) - 0x40000000) * 32 + ((ippin & 0x1f) * 4);
		paddr =  BitBandPeriAddr((void *)(GPIO_REG_BASE + GPIO_PORTB_DR * (ippin >> 5)),  ippin & 0x1f);
	}
	return paddr;
}

volatile uint8_t * GetInPinBitBandAddr(PinName pin) {
	volatile uint8_t * paddr = NULL;
	uint32_t ippin = HAL_GPIO_GetIPPinName_8195a(pin);
	if(ippin < 0xff) {
		// paddr = 0x42000000 + (0x40001000 + 0x0c * (ippin >> 5) - 0x40000000) * 32 + ((ippin & 0x1f) * 4);
		paddr = BitBandPeriAddr((void *)(GPIO_REG_BASE + GPIO_EXT_PORTA + (ippin >> 5) * 4),  ippin & 0x1f);
	}
	return paddr;
}

volatile uint8_t * HardSetPin(PinName pin, HAL_GPIO_PIN_MODE pmode, uint8_t val)
{
	volatile uint8_t *paddr = NULL;
	uint32_t ippin = HAL_GPIO_GetIPPinName_8195a(pin);
	if(ippin < 0xff) {
		if(_pHAL_Gpio_Adapter == NULL) {
			extern HAL_GPIO_ADAPTER gBoot_Gpio_Adapter;
			_pHAL_Gpio_Adapter = &gBoot_Gpio_Adapter;
		}
		if(_pHAL_Gpio_Adapter->Gpio_Func_En == 0) GPIO_FuncOn_8195a();
		wait_us(100);
		// delayMicroseconds(100);
		// paddr = 0x42000000 + (0x40001000 + 0x0c * (ippin >> 5) - 0x40000000) * 32 + ((ippin & 0x1f) * 4);
#if CONFIG_DEBUG_LOG > 3		
		GpioFunctionChk(ippin, ENABLE);
#endif		
    	GPIO_PullCtrl_8195a(ippin, HAL_GPIO_HIGHZ); // Make the pin pull control default as High-Z
		paddr = BitBandPeriAddr((void *)(GPIO_REG_BASE + GPIO_PORTB_DR * (ippin >> 5)),  ippin & 0x1f);
		*paddr = val;						// data register
		HAL_GPIO_PIN gpio;
		gpio.pin_name = ippin;
		gpio.pin_mode = pmode;
		HAL_GPIO_Init_8195a(&gpio);
		*paddr = val;						// data register
//		paddr[(GPIO_PORTB_DDR - GPIO_PORTB_DR) * 32] = pmode == DOUT_PUSH_PULL;	// data direction
//		GPIO_PullCtrl_8195a(ippin, pmode);					// set GPIO_PULL_CTRLx
//		paddr[(GPIO_PORTB_CTRL - GPIO_PORTB_DR) * 32] = 0;		// data source control, we should keep it as default: data source from software
	}
	return paddr;
}

