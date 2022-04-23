#include "Arduino.h"

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode) {
    if ( g_APinDescription[interruptNumber].ulPinType != PIO_GPIO_IRQ ) {
        pinRemoveMode(interruptNumber);
    }

    switch(mode) {
        case LOW:
            pinMode(interruptNumber, INPUT_IRQ_LOW);
            digitalSetIrqHandler(interruptNumber, (void *)callback);
            break;

        case HIGH:
            pinMode(interruptNumber, INPUT_IRQ_HIGH);
            digitalSetIrqHandler(interruptNumber, (void *)callback);
            break;

        case FALLING:
            pinMode(interruptNumber, INPUT_IRQ_FALL);
            digitalSetIrqHandler(interruptNumber, (void *)callback);
            break;

        case RISING:
            pinMode(interruptNumber, INPUT_IRQ_RISE);
            digitalSetIrqHandler(interruptNumber, (void *)callback);
            break;

        case CHANGE:
            // TODO
            break;
    }
}

void detachInterrupt(pin_size_t interruptNumber) {
    if ( g_APinDescription[interruptNumber].ulPinType == PIO_GPIO_IRQ ) {
        pinRemoveMode(interruptNumber);
    }
}
