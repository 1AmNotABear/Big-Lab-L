#ifndef CONTROL_SCREEN_H
#define CONTROL_SCREEN_H

typedef enum {
    CONTROL_IN_PROGRESS,
    CONTROL_SELECTED_HOME
} ControlResult;

/* Lets the logged-in admin (currentUser) adjust tempHighLimit and
   tempLowLimit with +/- buttons. HOME is the only way off this screen. */
ControlResult control_screen_step(void);

#endif
