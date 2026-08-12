#ifndef BLIND_SCREEN_H
#define BLIND_SCREEN_H

typedef enum {
    BLIND_IN_PROGRESS,
    BLIND_SELECTED_HOME
} BlindResult;

/* Lets the user set each blind to OPEN/MID/CLOSE via on-screen buttons that
   turn yellow when selected. State lives in homeState.blind1/blind2
   (src/homestate.h) so it persists across visits to this screen. */
BlindResult blind_screen_step(void);

#endif
