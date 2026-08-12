#ifndef BLINDS_SCREEN_H
#define BLINDS_SCREEN_H

typedef enum {
    BLINDS_IN_PROGRESS,
    BLINDS_SELECTED_HOME
} BlindsResult;

/* Lets the user change blind settins between open, midway, and closed for two blinds. State lives in homeState.blind1 and blind2
   (src/homestate.h) so it persists across visits to this screen. */
BlindsResult blindss_screen_step(void);

#endif