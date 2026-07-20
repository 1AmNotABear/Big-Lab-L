#include "lpc24xx.h"

#define LADDER_MASK  (0xFF << 1)   // LED ladder, P2.1 - P2.8
#define LADDER_ENABLE (1 << 22)    // LED ladder enable, P0.22
#define TRICOLOUR_MASK (0x3F << 16) // blind tricolour LEDs, P3.16 - P3.21

void setDirections(void) {
    FIO2DIR |= LADDER_MASK;    // LED ladder data pins as outputs
    FIO0DIR |= LADDER_ENABLE;  // LED ladder enable pin as output
    FIO3DIR |= TRICOLOUR_MASK; // blind tricolour LED pins as outputs

    // Add more peripheral direction setup here as they're added,
    // e.g. doorbell button (input), smart plug button (input)
}

void enablePeripherals(void) {
    FIO0SET = LADDER_ENABLE;   // drive the ladder enable pin high, permanently on
}
