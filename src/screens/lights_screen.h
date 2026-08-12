#ifndef LIGHTS_SCREEN_H
#define LIGHTS_SCREEN_H

typedef enum {
    LIGHTS_IN_PROGRESS,
    LIGHTS_SELECTED_HOME
} LightsResult;

/* Lets the user toggle each room light (and ALL ON / ALL OFF) via on-screen
   buttons that turn yellow when on. State lives in homeState.roomLights
   (src/homestate.h) so it persists across visits to this screen. */
LightsResult lights_screen_step(void);

#endif
