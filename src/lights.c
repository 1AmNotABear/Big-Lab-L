#include "lpc24xx.h"
#include "lights.h"

/* LED ladder is wired to P2.1 - P2.8 (8 pins)*/
#define LADDER_MASK (0xFF << 1)   // covers P2.1 - P2.8

void updateLightState(HomeState *state) {
    unsigned long ladderBits = ((unsigned long)state->roomLights << 1) & LADDER_MASK;

    FIO2CLR = LADDER_MASK;   // clear all ladder pins first
    FIO2SET = ladderBits;    // set only the pins that should be on
}