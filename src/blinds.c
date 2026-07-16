#include "lpc24xx.h"
#include "blinds.h"

// Defining the pins for each LED color light
#define LED1_RED (1 << 16) // P3.16
#define LED1_GREEN (1 << 17) // P3.17
#define LED1_BLUE (1 << 18) // P3.18

#define LED2_RED (1 << 19) // P3.19
#define LED2_GREEN (1 << 20) // P3.20
#define LED2_BLUE (1 << 21) // P3.21

/* triColour LEDs are wired to P3.16 - P3.21 (6 pins)*/
#define TRICOLOUR_MASK (0x3F << 16)   // covers P3.16 - P2.21

void updateBlindState(HomeState *state) {

    unsigned long triColourBits = 0; // stores the GPIO outputs to be switched on


    if (state->blind1 == BLIND_ROLLED_UP) {              // turns LED1 Red
        triColourBits |= LED1_RED;
    } else if (state->blind1 == BLIND_MID_WAY) {         // turns LED1 Green
        triColourBits |= LED1_GREEN;
    } else if (state->blind1 == BLIND_ROLLED_DOWN) {     // turns LED1 Blue
        triColourBits |= LED1_BLUE;
    }


    if (state->blind2 == BLIND_ROLLED_UP) {              // turns LED2 Red
        triColourBits |= LED2_RED;
    } else if (state->blind2 == BLIND_MID_WAY) {         // turns LED2 Green
        triColourBits |= LED2_GREEN;
    } else if (state->blind2 == BLIND_ROLLED_DOWN) {     // turns LED2 Blue
        triColourBits |= LED2_BLUE;
    }

    FIO3CLR = TRI_COLOUR_MASK;   // clear all tricolour pins first
    FIO3SET = triColourBits;    // set only the pins that should be on
}