#include "lpc24xx.h"

#define TRICOLOUR_MASK (0x3F << 16) // blind tricolour LEDs, P3.16 - P3.21

void setDirections(void) {
    FIO3DIR |= TRICOLOUR_MASK; // blind tricolour LED pins as outputs
}

void enablePeripherals(void) {
}
