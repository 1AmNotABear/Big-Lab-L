#include "lpc24xx.h"
#include "blinds.h"
#include "ADC.h"
#include "temperature.h"

// temp is read_temp()'s 0-99 scale, light is readADC()'s raw 0-1023 value
#define TEMP_COLD_MAX   15
#define TEMP_HOT_MIN    24
#define LIGHT_LOW_MAX   80
#define LIGHT_HIGH_MIN  150

// Defining the pins for each LED color light
#define LED1_RED (1 << 16) // P3.16
#define LED1_GREEN (1 << 17) // P3.17
#define LED1_BLUE (1 << 18) // P3.18

#define LED2_RED (1 << 19) // P3.19
#define LED2_GREEN (1 << 20) // P3.20
#define LED2_BLUE (1 << 21) // P3.21

/* triColour LEDs are wired to P3.16 - P3.21 (6 pins)*/
#define TRICOLOUR_MASK (0x3F << 16)   // covers P3.16 - P3.21

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

    FIO3CLR = TRICOLOUR_MASK;   // clear all tricolour pins first
    FIO3SET = triColourBits;    // set only the pins that should be on
}

// energy-saving rule table: cold+bright -> UP, cold+dark -> DOWN,
// hot+dim -> UP, hot+mid -> MID. Every other combination (comfortable
// temp range, or a light level with no matching rule) leaves the blind
// where it is.
static BlindPosition decideBlindPosition(int temp, unsigned int light, BlindPosition current)
{
    if (temp <= TEMP_COLD_MAX) {
        if (light >= LIGHT_HIGH_MIN) return BLIND_ROLLED_UP;
        if (light < LIGHT_LOW_MAX) return BLIND_ROLLED_DOWN;
        return current;
    }

    if (temp >= TEMP_HOT_MIN) {
        if (light < LIGHT_LOW_MAX) return BLIND_ROLLED_UP;
        if (light < LIGHT_HIGH_MIN) return BLIND_MID_WAY;
        return current;
    }

    return current;
}

void checkBlindAutomation(HomeState *state)
{
    int temp = read_temp();
    unsigned int light = readADC();

    if (!state->blind1Override)
        state->blind1 = decideBlindPosition(temp, light, state->blind1);

    if (!state->blind2Override)
        state->blind2 = decideBlindPosition(temp, light, state->blind2);

    updateBlindState(state);
}

