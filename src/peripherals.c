#include "lpc24xx.h"

#define LADDER_MASK  (0xFF << 1)   // LED ladder, P2.1 - P2.8
#define LADDER_ENABLE (1 << 22)    // LED ladder enable, P0.22
#define TRICOLOUR_MASK (0x3F << 16) // blind tricolour LEDs, P3.16 - P3.21

void setDirections(void) {
    FIO2DIR |= LADDER_MASK;    // LED ladder data pins as outputs
    IODIR0  |= LADDER_ENABLE;  // LED ladder enable pin (P0.22) as output - legacy regs, port 0 is in legacy mode (GPIOM=0)
    FIO3DIR |= TRICOLOUR_MASK; // blind tricolour LED pins as outputs

    // Add more peripheral direction setup here as they're added,
    // e.g. doorbell button (input), smart plug button (input)
}

void enablePeripherals(void) {
    IOSET0 = LADDER_ENABLE;    // drive the ladder enable pin (P0.22) high, permanently on - legacy regs (GPIOM=0)
}
