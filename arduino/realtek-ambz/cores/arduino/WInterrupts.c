
#include "WInterrupts.h"

void attachInterrupt(uint32_t pin, void (*callback)(void), uint32_t mode) {
    if ( g_APinDescription[pin].ulPinType != PIO_GPIO_IRQ ) {
        pinRemoveMode(pin);
        
    }

    switch(mode) {
        case LOW:
            pinMode(pin, INPUT_IRQ_LOW);
            digitalSetIrqHandler(pin, (void *)callback);
            break;

        case HIGH:
            pinMode(pin, INPUT_IRQ_HIGH);
            digitalSetIrqHandler(pin, (void *)callback);
            break;

        case FALLING:
            pinMode(pin, INPUT_IRQ_FALL);
            digitalSetIrqHandler(pin, (void *)callback);
            break;

        case RISING:
            pinMode(pin, INPUT_IRQ_RISE);
            digitalSetIrqHandler(pin, (void *)callback);
            break;

        case CHANGE:
            // TODO
            break;
    }
}

void detachInterrupt(uint32_t pin) {
    if ( g_APinDescription[pin].ulPinType == PIO_GPIO_IRQ ) {
        pinRemoveMode(pin);
    }
}
