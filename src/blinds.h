#ifndef BLINDS_H
#define BLINDS_H

#include "homestate.h"

void updateBlindState(HomeState *state);

// reads the temp/light sensors and moves any blind that isn't manually
// overridden, then calls updateBlindState() to sync the LEDs
void checkBlindAutomation(HomeState *state);

#endif
