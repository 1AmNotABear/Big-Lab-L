#include "lpc24xx.h"

#define TRICOLOUR_MASK (0x3F << 16) // blind tricolour LEDs, P3.16 - P3.21

void setDirections(void) {
    FIO3DIR |= TRICOLOUR_MASK; // blind tricolour LED pins as outputs

    // Add more peripheral direction setup here as they're added,
    // e.g. doorbell button (input), smart plug button (input)
}

void enablePeripherals(void) {
    // nothing to enable currently
}
